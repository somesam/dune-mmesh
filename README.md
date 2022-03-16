# The Dune-MMesh Module

The Dune-MMesh Module is an implementation of the DUNE grid interface that wraps CGAL triangulations in 2D and 3D. It is also capable to export a prescribed set of cell facets as a dim-1 interface grid and remesh the grid when moving this interface.

You find the full documentation of Dune-MMesh at [dune-mmesh.readthedocs.io](https://dune-mmesh.readthedocs.io).


## Installation

Note that Dune-MMesh has a list of dependencies: C++ compiler, CMake, Python3 + pip (+ venv), pkg-config, Boost, OpenMPI, SuiteSparse, Gmsh.

A minimal setup with Docker can be set up as follows:
````
docker run -it ubuntu:latest
apt update
apt install g++ cmake python3 python3-pip python3-venv pkg-config libboost-dev libopenmpi3 libsuitesparse-dev gmsh git
````

On MacOS, you can install the required dependencies with Xcode Command Line Tools and Homebrew:
````
xcode-select --install
brew install pkg-config boost openmpi suite-sparse gmsh
````

We strongly recommend using a virtual environment:
````
python3 -m venv dune-env
source dune-env/bin/activate
````

Install the Dune-MMesh package using pip:
````
pip install dune-mmesh
````
Note that this takes some time in order to compile all dependent Dune modules.

Now, you should be able to execute Dune-MMesh's python code. For instance:
````
git clone https://github.com/samuelburbulla/dune-mmesh.git
cd dune-mmesh/doc/examples/grids
python horizontal.py
cd ..
python coupling.py
````

Further details on the installation procedure can be found on [Installation](https://dune-mmesh.readthedocs.io/en/latest/installation.html).


## Example

You can find a collection of examples of how to use Dune-MMesh on our [Examples](https://dune-mmesh.readthedocs.io/en/latest/examples.html) page.


In a short example below we show how to use Dune-MMesh in general.
We use `.msh` files generated by gmsh to define the geometry of our mesh.
````
import gmsh
gmsh.initialize()
gmsh.option.setNumber("Mesh.MshFileVersion", 2.2)
name = "grid.msh"
h = 0.01
gmsh.model.add(name)
kernel = gmsh.model.occ
box = kernel.addRectangle(0, 0, 0, 1, 1)
p0 = kernel.addPoint(0.5, 0.5, 0, h)
p1 = kernel.addPoint(0.25, 0.25, 0, h)
p2 = kernel.addPoint(0.75, 0.75, 0, h)
p3 = kernel.addPoint(0.75, 0.25, 0, h)
lf1 = kernel.addLine(p0, p1)
lf2 = kernel.addLine(p0, p2)
lf3 = kernel.addLine(p0, p3)
kernel.synchronize()
gmsh.model.mesh.embed(1, [lf1, lf2, lf3], 2, box)
gmsh.model.mesh.generate(dim=2)
gmsh.write(name)
gmsh.finalize()
````

Read the generated `.msh` file and construct bulk and interface triangulation.
````
from dune.grid import reader
from dune.mmesh import mmesh
gridView  = mmesh((reader.gmsh, name), 2)
igridView = gridView.hierarchicalGrid.interfaceGrid
````

Now, we can solve a mixed-dimensional PDE.
````
from dune.mmesh import trace, skeleton, interfaceIndicator, monolithicSolve

from dune.fem.space import dglagrange, lagrange
space = dglagrange(gridView, order=3)
ispace = lagrange(igridView, order=3)

from ufl import *
u = TrialFunction(space)
v = TestFunction(space)
n = FacetNormal(space)
h = MinFacetEdgeLength(space)
uh = space.interpolate(0, name="uh")

iu = TrialFunction(ispace)
iv = TestFunction(ispace)
iuh = ispace.interpolate(0, name="iuh")

from dune.mmesh import interfaceIndicator
I = interfaceIndicator(igridView)

from dune.ufl import Constant
q = Constant(1, name="q")
beta = Constant(1e2, name="beta")
omega = Constant(1e-6, name="omega")

a  = inner(grad(u), grad(v)) * dx
a += beta / h * inner(jump(u), jump(v)) * (1-I)*dS
a -= dot(dot(avg(grad(u)), n('+')), jump(v)) * (1-I)*dS

a += beta / h * inner(u - 0, v) * ds
a -= dot(dot(grad(u), n), v) * ds

ia  = inner(grad(iu), grad(iv)) * dx
ib  = q * iv * dx

from dune.mmesh import skeleton, trace
a -= (skeleton(iuh)('+') - u('+')) / omega * v('+') * I*dS
a -= (skeleton(iuh)('-') - u('-')) / omega * v('-') * I*dS

ia += (iu - trace(uh)('+')) / omega * iv * dx
ia += (iu - trace(uh)('-')) / omega * iv * dx

from dune.fem.scheme import galerkin
scheme = galerkin([a == 0])
ischeme = galerkin([ia == ib])

from dune.mmesh import monolithicSolve
monolithicSolve(schemes=(scheme, ischeme), targets=(uh, iuh), verbose=True)
````

We can write the solution to `.vtk` or plot with matplotlib.
````
gridView.writeVTK("example", pointdata=[uh], nonconforming=True)
igridView.writeVTK("interface", pointdata=[iuh])

import matplotlib.pyplot as plt
from dune.fem.plotting import plotPointData as plot
fig = plt.figure()
plot(uh, linewidth=0, clim=[0, 0.17], figure=fig)
plot(iuh, linewidth=0.01, colorbar=None, clim=[0, 0.17], figure=fig)
plt.savefig("plot.png")
````

![](scripts/plot.png)

## Testing
You can test your installation of Dune-MMesh by running the python tests
````
python -m dune.mmesh test
````
Further tests of the C++ backend can be performed with a [source build](https://dune-mmesh.readthedocs.io/en/latest/installation.html#from-source) executing `make build_test` and `make test` in the build directory.

## Contribution

Contributions are highly welcome. If you want to contribute, please use our [GitLab](https://gitlab.dune-project.org/samuel.burbulla/dune-mmesh) or [GitHub](https://github.com/samuelburbulla/dune-mmesh/).

## License
Dune-MMesh is licensed under the terms and conditions of the GNU General Public License (GPL) version 3 or - at your option - any later version.
