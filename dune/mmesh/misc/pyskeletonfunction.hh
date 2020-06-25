#ifndef DUNE_MMESH_PYSKELETONFUNCTION
#define DUNE_MMESH_PYSKELETONFUNCTION

#if HAVE_DUNE_FEM

#include <dune/common/exceptions.hh>
#include <dune/fem/common/intersectionside.hh>
#include <dune/fem/function/localfunction/const.hh>
#include <dune/fem/function/localfunction/bindable.hh>
#include <dune/fempy/py/grid/gridpart.hh>
#include <dune/fempy/py/grid/function.hh>

template <class BulkGV, class InterfaceGridFunction>
struct SkeletonGF
: public Dune::Fem::BindableGridFunctionWithSpace<Dune::FemPy::GridPart<BulkGV>,
               typename InterfaceGridFunction::RangeType>
{
  using GridPart = Dune::FemPy::GridPart<BulkGV>;
  using Base = Dune::Fem::BindableGridFunctionWithSpace<GridPart,typename InterfaceGridFunction::RangeType>;
  using ILocalFunction = Dune::Fem::ConstLocalFunction<InterfaceGridFunction>;
  SkeletonGF(const BulkGV &bulkGV, const InterfaceGridFunction &igf)
  : Base(Dune::FemPy::gridPart<BulkGV>(bulkGV), "interface_"+igf.name(), igf.order() ),
    ilf_(igf), onInterface_(false), name_(igf.name())
    {}
  void bind(const typename Base::EntityType &entity)
  {
    Base::bind(entity);
  }
  void bind(const typename BulkGV::Intersection &intersection,
            Dune::Fem::IntersectionSide side)
  {
    if( Base::gridPart().grid().isInterface( intersection ) )
    {
      onInterface_ = true;
      ilf_.bind( Base::gridPart().grid().asInterfaceEntity( intersection ) );
      // need to store something about the `geometryInInside/Outside`
      // depending on side
    }
    else onInterface_ = false;
  }
  template <class Point>
  void evaluate(const Point &x, typename Base::RangeType &ret) const
  {
    if (onInterface_)
    {
      auto xLocal = Dune::Fem::coordinate(x);
      // TODO auto ix     = sideGeometry_.local( xLocal );
      typename InterfaceGridFunction::LocalCoordinateType ix{0.5};
      ilf_.evaluate(ix,ret);
    } else ret = typename Base::RangeType(0);
  }
  // the following could be implemented using the tangential derivatives of
  // the interface function and assuing constant in normal direction
  template <class Point>
  void jacobian(const Point &x, typename Base::JacobianRangeType &ret) const
  {
    // TODO
    DUNE_THROW( Dune::NotImplemented, "SkeletonFunction::jacobian not implemented" );
  }
  template <class Point>
  void hessian(const Point &x, typename Base::HessianRangeType &ret) const
  {
    // TODO
    DUNE_THROW( Dune::NotImplemented, "SkeletonFunction::hessian not implemented" );
  }
  // unsigned int order() const { return order_; }
  // std::string name() const { return "interface_"+name_; } // needed for output
  private:
  ILocalFunction ilf_;
  bool onInterface_;
  const std::string name_;
  int order_;
};

template <class IGV, class BulkGridFunction, Dune::Fem::IntersectionSide side>
struct TraceGF
: public Dune::Fem::BindableGridFunctionWithSpace<Dune::FemPy::GridPart<IGV>,
               typename BulkGridFunction::RangeType>
{
  using GridPart = Dune::FemPy::GridPart<IGV>;
  using Base = Dune::Fem::BindableGridFunctionWithSpace<GridPart,typename BulkGridFunction::RangeType>;
  using BLocalFunction = Dune::Fem::ConstLocalFunction<BulkGridFunction>;
  TraceGF(const IGV &iGV, const BulkGridFunction &bgf)
  : Base(Dune::FemPy::gridPart<IGV>(iGV), "trace_"+bgf.name(), bgf.order() ),
    blf_(bgf)
  {}
  void bind(const typename Base::EntityType &entity)
  {
    Base::bind(entity);
    const auto intersection = Base::gridPart().grid().getMMesh().asIntersection( entity );
    if constexpr (side == Dune::Fem::IntersectionSide::in)
      blf_.bind(intersection.inside());
    else if (intersection.neighbor())
      blf_.bind(intersection.outside());
    else // is this the best we can do?
      blf_.bind(intersection.inside());
  }
  template <class Point>
  void evaluate(const Point &x, typename Base::RangeType &ret) const
  {
    // again need to transfer the x (in this case on the interface) to an x in bulk
    auto xLocal = Dune::Fem::coordinate(x);
    typename BulkGridFunction::LocalCoordinateType bx{1./3.,1./3.};
    blf_.evaluate(bx,ret);
  }
  // need to implement jacobian and hessian as their tangential components
  template <class Point>
  void jacobian(const Point &x, typename Base::JacobianRangeType &ret) const
  {
    // again need to transfer the x (in this case on the interface) to an x in bulk
    auto xLocal = Dune::Fem::coordinate(x);
    typename BulkGridFunction::LocalCoordinateType bx{1./3.,1./3.};
    typename BulkGridFunction::JacobianRangeType bulkJac;
    blf_.jacobian(bx,bulkJac);
    ret = bulkJac; // could decide to remove normal component - but perhaps don't have to?
  }
  template <class Point>
  void hessian(const Point &x, typename Base::HessianRangeType &ret) const
  {
    // TODO
    DUNE_THROW( Dune::NotImplemented, "TraceFunction::hessian not implemented" );
  }
  private:
  BLocalFunction blf_;
};

#endif // HAVE_DUNE_FEM

#endif // DUNE_MMESH_PYSKELETONFUNCTION
