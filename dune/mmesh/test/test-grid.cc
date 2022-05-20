// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <iostream>
#include <dune/common/parallel/mpihelper.hh> // An initializer of MPI
#include <dune/common/exceptions.hh> // We use exceptions

#include <dune/grid/test/gridcheck.hh>
#include <dune/grid/test/checktwists.hh>

#include <dune/mmesh/mmesh.hh>

using namespace Dune;

/** Test-template main program. Instantiate a single expression
 * template and evaluate it on a simple grid.
 */
int main(int argc, char *argv[])
{
  MPIHelper::instance(argc, argv);

  std::cout << "-- Grid check --" << std::endl;

  // Create Grid
  // ------------
  static constexpr int dim = GRIDDIM;
  using Grid = Dune::MovingMesh<dim>;

  using GridFactory = Dune::GmshGridFactory< Grid >;
  GridFactory gridFactory( (dim == 2) ? "grids/line2d.msh" : "grids/plane3d.msh" );

  Grid& grid = *gridFactory.grid();
  const auto& igrid = grid.interfaceGrid();
  grid.loadBalance();

  std::cout << "Rank " << grid.comm().rank() << ": Elements " << grid.size(0) << " (" << grid.ghostSize(0) << " ghost)" << std::endl;
  std::cout << "Rank " << grid.comm().rank() << ": Facets " << grid.size(1) << " (" << grid.ghostSize(1) << " ghost)" << std::endl;
  std::cout << "Rank " << grid.comm().rank() << ": Vertices " << grid.size(2) << " (" << grid.ghostSize(2) << " ghost)" << std::endl;

  std::cout << "Interface Rank " << grid.comm().rank() << ": Elements " << igrid.size(0) << " (" << igrid.ghostSize(0) << " ghost)" << std::endl;
  std::cout << "Interface Rank " << grid.comm().rank() << ": Vertices " << igrid.size(1) << " (" << igrid.ghostSize(1) << " ghost)" << std::endl;

  // Call gridcheck from dune-grid
  gridcheck( grid );

  // Call grid check for interface grid
  gridcheck( igrid );

  VTKWriter vtkWriter( grid.leafGridView() );
  vtkWriter.write("test-grid");

  VTKWriter ivtkWriter( igrid.leafGridView() );
  ivtkWriter.write("test-igrid");

  return EXIT_SUCCESS;
}
