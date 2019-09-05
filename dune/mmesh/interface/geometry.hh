// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MMESH_INTERFACE_GEOMETRY_HH
#define DUNE_MMESH_INTERFACE_GEOMETRY_HH

/** \file
 * \brief The MMeshInterfaceGridGeometry class and its specializations
 * Inherits from Dune::AffineGeometry.
 */

// Dune includes
#include <dune/common/fmatrix.hh>
#include <dune/common/typetraits.hh>
#include <dune/grid/common/geometry.hh>
#include <dune/geometry/affinegeometry.hh>

// CGAL includes
#include <CGAL/Kernel/global_functions.h>

// local includes
#include <dune/mmesh/grid/pointfieldvector.hh>

namespace Dune
{

  /** \brief Geometry
   */

  template<int mydim, int coorddim, class GridImp>
  class MMeshInterfaceGridGeometry {};

  //! 2D Geometry

  template<int mydim, class GridImp>
  class MMeshInterfaceGridGeometry<mydim, 2, GridImp> :
    public AffineGeometry <typename GridImp::ctype, mydim, 2>
  {
    static constexpr int coorddim = 2;
    typedef AffineGeometry <typename GridImp::ctype, mydim, coorddim> BaseType;
    typedef FieldVector<typename GridImp::ctype, coorddim> FVector;

  public:
    enum { dimension = GridImp::dimension };
    enum { dimensionworld = GridImp::dimensionworld };
    enum { coorddimension = coorddim };
    enum { mydimension = mydim };

    //! Constructor from host geometry with codim 0
    MMeshInterfaceGridGeometry(const typename GridImp::template MMeshInterfaceEntity<0>& hostEntity)
     : BaseType( GeometryTypes::simplex(mydim),
         std::array<FVector, 2>( {
           makeFieldVector( hostEntity.first->vertex( (hostEntity.second+1)%3 )->point() ),
           makeFieldVector( hostEntity.first->vertex( (hostEntity.second+2)%3 )->point() )
       } ) )
    {
        circumcenter_ = this->corner(0);
        circumcenter_ += this->corner(1);
        circumcenter_ *= 0.5;
    }

    //! Constructor from host geometry with codim 0
    template< class Vertex >
    MMeshInterfaceGridGeometry(const std::array<Vertex, 2>& vertices)
     : BaseType( GeometryTypes::simplex(mydim),
         std::array<FVector, 2>( {
           makeFieldVector( vertices[0].point() ),
           makeFieldVector( vertices[1].point() )
       } ) )
    {
        circumcenter_ = this->corner(0);
        circumcenter_ += this->corner(1);
        circumcenter_ *= 0.5;
    }

    //! Constructor from host geometry with codim 1
    MMeshInterfaceGridGeometry(const typename GridImp::template MMeshInterfaceEntity<1>& hostEntity)
     : BaseType( GeometryTypes::simplex(mydim), std::array<FVector, 1>( { makeFieldVector( hostEntity->point() ) } ) ),
       circumcenter_( this->corner(0) )
    {}

    /** \brief Obtain the circumcenter */
    const FVector circumcenter () const
    {
      return circumcenter_;
    }

  private:
    FVector circumcenter_;
  };

  //! 3D Geometry

  template<int mydim, class GridImp>
  class MMeshInterfaceGridGeometry<mydim, 3, GridImp> :
    public AffineGeometry <typename GridImp::ctype, mydim, 3>
  {
    static constexpr int coorddim = 3;
    typedef AffineGeometry <typename GridImp::ctype, mydim, coorddim> BaseType;
    typedef typename GridImp::ctype ctype;
    typedef FieldVector<ctype, coorddim> FVector;

  public:
    enum { dimension = GridImp::dimension };
    enum { dimensionworld = GridImp::dimensionworld };
    enum { coorddimension = coorddim };
    enum { mydimension = mydim };

    //! Constructor from host geometry with codim 0
    MMeshInterfaceGridGeometry(const typename GridImp::template MMeshInterfaceEntity<0>& hostEntity)
     : BaseType( GeometryTypes::simplex(mydim), getVertices<0>(hostEntity) )
    {
      // obtain circumcenter
      const auto& cell     = hostEntity.first;
      const auto& facetIdx = hostEntity.second;

      // use the CGAL index convention to obtain the vertices
      circumcenter_ = makeFieldVector(
        CGAL::circumcenter(
          cell->vertex( (facetIdx + 1) % 4 )->point(),
          cell->vertex( (facetIdx + 2) % 4 )->point(),
          cell->vertex( (facetIdx + 3) % 4 )->point()
        )
      );
    }

    //! Constructor from host geometry with codim 0
    template< class Vertex >
    MMeshInterfaceGridGeometry(const std::array<Vertex, 3>& vertices)
     : BaseType( GeometryTypes::simplex(mydim),
         std::array<FVector, 3>( {
           makeFieldVector( vertices[0].point() ),
           makeFieldVector( vertices[1].point() ),
           makeFieldVector( vertices[2].point() )
       } ) )
    {
      // obtain circumcenter
      circumcenter_ = makeFieldVector(
        CGAL::circumcenter(
          vertices[0].point(),
          vertices[1].point(),
          vertices[2].point()
        )
      );
    }

    //! Constructor from host geometry with codim 1
    MMeshInterfaceGridGeometry(const typename GridImp::template MMeshInterfaceEntity<1>& hostEntity)
     : BaseType( GeometryTypes::simplex(mydim), getVertices<1>( hostEntity ) )
    {
      // obtain circumcenter
      circumcenter_ = this->corner(0);
      circumcenter_ += this->corner(1);
      circumcenter_ *= 0.5;
    }

    //! Constructor from host geometry with codim 2
    MMeshInterfaceGridGeometry(const typename GridImp::template MMeshInterfaceEntity<2>& hostEntity)
     : BaseType( GeometryTypes::simplex(mydim), std::array<FVector, 1>( { makeFieldVector( hostEntity->point() ) } ) ),
                 circumcenter_( this->corner(0) )
    {}

    /** \brief Obtain the circumcenter */
    const FVector circumcenter () const
    {
      return circumcenter_;
    }

  private:

     template< int codim, typename Enable = std::enable_if_t< codim == 0 > >
     static inline std::array<FVector, 3> getVertices ( const typename GridImp::template MMeshInterfaceEntity<0>& hostEntity )
     {
       std::array<FVector, 3> vertices;

       const auto& cell     = hostEntity.first;
       const auto& facetIdx = hostEntity.second;

       // use the CGAL index convention to obtain the vertices
       for ( int i = 0; i < 3; ++i )
         vertices[i] = makeFieldVector( cell->vertex( (facetIdx + i + 1) % 4 )->point() );

       return vertices;
     }

     template< int codim, typename Enable = std::enable_if_t< codim == 1 > >
     static inline std::array<FVector, 2> getVertices ( const typename GridImp::template MMeshInterfaceEntity<1>& hostEntity )
     {
       std::array<FVector, 2> vertices;

       const auto& cell       = hostEntity.first;
       const auto& vertexIdx1 = hostEntity.second;
       const auto& vertexIdx2 = hostEntity.third;

       vertices[0] = makeFieldVector( cell->vertex( vertexIdx1 )->point() );
       vertices[1] = makeFieldVector( cell->vertex( vertexIdx2 )->point() );

       return vertices;
     }

    FVector circumcenter_;
  };

}  // namespace Dune

#endif
