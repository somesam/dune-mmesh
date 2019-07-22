// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

// stl includes
#include <iostream>

// dune-common includes
#include <dune/common/exceptions.hh>

// dune-grid includes
#include <dune/grid/io/file/vtk/vtkwriter.hh>

// dune-mmesh includes
#include <dune/mmesh/mmesh.hh>

/** MMesh gmsh example main program.
 */
int main( int argc, char *argv[] )
{
  try {
    // get the grid dimension from compile definitions
    static constexpr int dim = GRIDDIM;

    // print some status
    std::cout << "Create MMesh from grids/cube" + std::to_string(dim) + "d.msh..." << std::endl;

    // type of MMesh grid implementation
    using Grid = Dune::MovingMesh< dim >;

    // create a grid pointer from .msh file
    using GridFactory = Dune::GmshGridFactory< Grid, /*useExplicitGridFactory=*/false >;
    GridFactory gridFactory( "grids/cube" + std::to_string(dim) + "d.msh" );

    // get grid reference
    Grid& grid = *gridFactory.grid();

    // print the number of cells
    std::cout << "Number of cells: " << grid.leafGridView().size(0) << std::endl;

    // print some status
    std::cout << "Write grid into gmsh-grid-" + std::to_string(dim) + "d.vtu... " << std::endl;

    // write grid to vtk file
    Dune::VTKWriter<typename Grid::LeafGridView> vtkwriter( grid.leafGridView() );
    vtkwriter.write( "gmsh-grid-" + std::to_string(dim) + "d", Dune::VTK::ascii );

    return EXIT_SUCCESS;
  }
  catch (Dune::Exception &e){
    std::cerr << "Dune reported error: " << e << std::endl;
    return EXIT_FAILURE;
  }
  catch (CGAL::Failure_exception &e){
    std::cerr << "CGAL reported error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(...) {
    std::cerr << "Unknown exception thrown!" << std::endl;
    return EXIT_FAILURE;
  }
}
