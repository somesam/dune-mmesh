// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_MMESH_INTERFACE_GRIDFACTORY_HH
#define DUNE_MMESH_INTERFACE_GRIDFACTORY_HH

// Dune includes
#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/common/boundarysegment.hh>

namespace Dune
{

  /** \brief specialization of the GridFactory for MMesh InterfaceGrid
   *
   *  \ingroup GridFactory
   */

  //! The grid factory for MMesh InterfaceGrid
  template< class MMeshImp, int dim >
  class GridFactory< MMeshInterfaceGrid<MMeshImp, dim> >
    : public GridFactoryInterface< MMeshInterfaceGrid<MMeshImp, dim> >
  {
    typedef GridFactory< MMeshInterfaceGrid<MMeshImp, dim> > This;

  public:
    //! type of interface grid
    typedef MMeshInterfaceGrid<MMeshImp, dim> Grid;

    //! type of (scalar) coordinates
    typedef typename Grid::ctype ctype;
    typedef typename Grid::HostGridType HostGrid;

    //! type of corresponding mmesh
    typedef typename Grid::MMesh MMesh;

    //! dimension of the grid
    static const int dimension = Grid::dimension;
    //! dimension of the world
    static const int dimensionworld = Grid::dimensionworld;

    //! type of vector for world coordinates
    typedef FieldVector< ctype, dimensionworld > WorldVector;
    //! type of matrix from world coordinates to world coordinates
    typedef FieldMatrix< ctype, dimensionworld, dimensionworld > WorldMatrix;

    typedef Dune::BoundarySegment< dimension, dimensionworld > BoundarySegment;
    typedef std::map< std::vector< std::size_t >, std::size_t > BoundarySegments;
    typedef std::map< std::vector< std::size_t >, unsigned int > InsertionIndexMap;

    typedef std::map< std::size_t, std::size_t > VertexIdMap;

    template< int codim >
    struct Codim
    {
      typedef typename Grid::template Codim< codim >::Entity Entity;
    };

  public:
    //! are boundary ids supported by this factory?
    static const bool supportsBoundaryIds = true;
    //! the factory is not able to create periodic meshes
    static const bool supportPeriodicity = false;

    /** default constructor */
    GridFactory ( const std::shared_ptr<MMesh> mMesh )
     : mMesh_( mMesh )
    {}

    /** \brief insert an element into the macro grid
     *
     *  \param[in]  type      GeometryType of the new element
     *  \param[in]  v         indices of the element vertices (starting with 0)
     */
    void insertElement ( const GeometryType &type,
                         const std::vector< unsigned int > &vertices )
    {
      // mark vertices as interface segment in host mmesh
      std::vector< std::size_t > ids;
      for ( const auto& v : vertices )
        ids.push_back( vertexIdMap_.at( v ) );

      std::sort(ids.begin(), ids.end());

      mMesh_->addInterfaceSegment( ids );

      insertionIndexMap_.insert( { ids, countElements++ } );
    };

    /** \brief insert a boundary segment into the macro grid
     *
     *  Only influences the ordering of the boundary segments
     *  \param[in]  vertices         vertex indices of boundary face
     */

    virtual void insertBoundarySegment ( const std::vector< unsigned int >& vertices )
    {
      std::vector< std::size_t > sorted_vertices;
      for ( const auto& v : vertices )
        sorted_vertices.push_back( vertexIdMap_.at( v ) );
      std::sort(sorted_vertices.begin(), sorted_vertices.end());

      if( boundarySegments_.find( sorted_vertices ) != boundarySegments_.end() )
        DUNE_THROW( GridError, "A boundary segment was inserted twice." );

      boundarySegments_.insert( std::make_pair( sorted_vertices, countBoundarySegments++ ) );
    }

    void insertBoundarySegment ( const std::vector< unsigned int >& vertices,
                                 const std::shared_ptr< BoundarySegment >& boundarySegment )
    {
      DUNE_THROW( NotImplemented, "insertBoundarySegments with Dune::BoundarySegment" );
    }

    /** \brief Insert a vertex into the macro grid
     *
     *  \param[in]  pos  position of the vertex (in world coordinates)
     *  \note This method assumes that the vertices are inserted consecutively
     *        with respect to their index.
     */
    void insertVertex ( const WorldVector &pos )
    {
      // insert the vertex to the host mmesh
      auto vh = mMesh_->getHostGrid().insert( makePoint( pos ) );
      // TODO: assert that this point was already inserted, otherwise we have to update mMesh!

      vertexIdMap_.insert( { countVertices, vh->info().id } );
      vh->info().isInterface = true;

      countVertices++;
    }

    /** \brief return insertion index of entity
     *
     *  \param[in]  entity  Entity of codim 0
     */
    unsigned int insertionIndex ( const typename Codim<0>::Entity &entity ) const
    {
      std::vector< std::size_t > ids;
      for( std::size_t i = 0; i < entity.subEntities(dimension); ++i )
        ids.push_back( entity.template subEntity<dimension>(i).impl().hostEntity()->info().id );
      std::sort(ids.begin(), ids.end());
      auto it = insertionIndexMap_.find( ids );
      if( it != insertionIndexMap_.end() )
        return it->second;
      else
        // return id // TODO the search above is not really what we want here every call
        return mMesh_->interfaceGrid().globalIdSet().id( entity );
    }

    /** \brief return insertion index of vertex entity
     *
     *  \param[in]  entity  Entity of codim dimension
     */
    unsigned int insertionIndex ( const typename Codim< dimension >::Entity &entity ) const
    {
      std::size_t index = mMesh_->interfaceGrid().globalIdSet().id( entity ); // TODO is this the right index?
      assert( index < std::numeric_limits<unsigned int>::max() );
      return index;
    }

    /** \brief finalize grid creation and hand over the grid
     *
     *  This version of createGrid is original to the MMesh grid factroy,
     *  allowing to specity a grid name.
     *
     *  \returns a pointer to the newly created grid
     *
     *  \note The caller takes responsibility of freeing the memory allocated
     *        for the grid.
     *  \note MMesh's grid factory provides a static method for freeing the
     *        grid (destroyGrid).
     */

    typename Grid::GridPtrType createGrid ()
    {
      DUNE_THROW( InvalidStateException, "The interface grid cannot be created, get the pointer by getGrid()!" );
      return nullptr;
    }

    auto getGrid ()
    {
      mMesh_->interfaceGridPtr()->setIndices();
      mMesh_->interfaceGridPtr()->setBoundarySegments( boundarySegments_ );

      // Return pointer to grid
      return mMesh_->interfaceGridPtr();
    }

    /** \brief destroy a grid previously obtained from this factory
     *
     *  \param[in]  grid  pointer to the grid to destroy
     */
    static void destroyGrid ( Grid *grid )
    {
      DUNE_THROW( InvalidStateException, "The interface grid cannot be destroyed, destroy the MMesh instead!" );
    }

  private:
    //! Private members
    std::shared_ptr<MMesh> mMesh_;
    BoundarySegments boundarySegments_;
    std::size_t countBoundarySegments = 0;
    VertexIdMap vertexIdMap_;
    InsertionIndexMap insertionIndexMap_;
    unsigned int countElements = 0;
    std::size_t countVertices = 0;
  };

} // end namespace Dune

#endif
