lc = 0.1;
lcf = 0.1;

// domain corners
Point(1) = {0,   0, 0, lc};
Point(2) = {0.5, 0, 0, lc};
Point(3) = {1,   0, 0, lcf};
Point(4) = {1,   1, 0, lc};
Point(5) = {0.5, 1, 0, lc};
Point(6) = {0,   1, 0, lcf};

// domain outline
Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 5};
Line(5) = {5, 6};
Line(6) = {6, 1};

Physical Line(1) = {1:6};

Line(10) = {2, 5};

// domain surface
Line Loop(1) = {1,10,5,6};
Line Loop(2) = {2,3,4,-10};
Plane Surface(1) = {1};
Plane Surface(2) = {2};
Physical Surface(0) = {1};
Physical Surface(1) = {2};

// interface
Line {10} In Surface{1};
Physical Line(10) = {10};

// interface boundary
Physical Point(1) = {2,5};
