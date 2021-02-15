The dune-mmesh module
=======================

The dune-mmesh module is an implementation of the DUNE grid interface that wraps CGAL triangulations in 2D and 3D.
It is also capable to export a prescribed set of cell facets as a dim-1 interface grid and remesh the grid when moving this interface.

Installation
------------

dune-mmesh requires the DUNE core modules, version 2.7 or later.

Additionally, you'll need a current version of `CGAL` installed on your system.

Examples
--------

See the `examples` folder for some code snippets or use `python -m dune.mmesh` to obtain a tutorial folder.

Creating a mesh in python using a .msh file works as follows:
````
from dune.mmesh import mmesh
from dune.grid import reader
gridView = mmesh((reader.gmsh, "grid.msh"), dim)
igridView = gridView.hierarchicalGrid.interfaceGrid
````

Remark that the .msh files must be built in the msh2 format, i.e for instance:

````gmsh -<dim> -format msh2 grid.geo````

Short C++ example using the GmshReader:
````
    #include <dune/mmesh/mmesh.hh>

    using Grid = Dune::MovingMesh< dim >;

    using GridFactory = Dune::GmshGridFactory< Grid >;
    GridFactory gridFactory( "grid.msh" );

    Grid& grid = *gridFactory.grid();

    using InterfaceGrid = typename Grid::InterfaceGrid;
    const InterfaceGrid& igrid = grid.interfaceGrid();
````

In addition, you can use a `.dgf` file or the `CGAL` triangulation interface directly.

Documentation
-------------

The complete class documentation generated by Doxygen can be found [here](https://www.dune-project.org/doxygen/dune-mmesh/master/).
