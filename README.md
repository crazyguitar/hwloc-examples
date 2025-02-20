# hwloc examples

```
# create an enroot sqush
make sqush

# launch a enroot environment for testing
name="test"
enroot --name "${name}" hwloc-test+latest.sqsh
enroot start --mount "/fsx:/fsx" "${name}" /bin/bash

# build hwloc example
make
./build/examples/hwloc-example

# remove enroot environment
enroot remove "${name}"
```
