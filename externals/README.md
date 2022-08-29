# External dependencies

dependencies.cmake lists all external dependencies that are installed. Some dependencies are also in the "pre_install" folder, these are pre-built dependencies.

To build the dependencies call `scripts/dependencies.sh`. This may take a while dependening on your system since opencv can take some time to be built. Everything will be installed to the "externals/install" folder.
