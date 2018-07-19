This is graywolf - a fork of TimberWolf 6.3.5

TimberWolf was developed at Yale University, and was distributed as open source
for a time until it was taken commercial. The last open-source version of
TimberWolf does not perform detail routing, but is a professional-grade
placement tool. In order to continue improving the open-source version,
graywolf has been forked off from version 6.3.5 of TimberWolf.

The main improvement in graywolf is that the build process is more streamlined
and that it behaves as a normal linux tool - you can call it from anywhere and
no environment variables must be set first.

What it does
------------

graywolf is used for placement in VLSI design. It's mainly used together with
qflow. (http://opencircuitdesign.com/qflow/)

Install procedure
-----------------
```
mkdir build
cd build
cmake ..
make  
sudo make install  
```

Test
----

After "make", you can run the test suite: (unfortunately, the test will not work on 32-bit architectures yet)

```
make test
```

To dump out log for failing tests:

```
CTEST_OUTPUT_ON_FAILURE=1 make test
```

To run completely verbose tests:

```
make test ARGS="-V"
```


Contributions
-------------

There are two main branches: *master* and *dev*. Since people expect *master*
to be stable in a "production environment", any ground-breaking
changes/refactoring must be merged into *dev*. Please do not open pull-request
towards *master* for these changes.

Pull requests targeting specific bugfixes need to be merged into both *master*
and *dev*, so it is ok to open a pull request for either of them.
