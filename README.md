# bgps-temporal-graphs

## Instructions

To run our code, **we have to install an extended version of the library SDSL**. Go to [this repository](https://github.com/darroyue/sdsl-lite) and follow the instructions.

After the extended version of SDSL is installed, we have to clone this repository and follow these steps:

1. Create our `build` folder and compile the code:
```Bash
mkdir build
cd build
cmake ..
make
```

Check that there is no errors.

2. Building the index. After compiling the code we should have an executable called `build-index` in `build`. Now run:

```Bash
./build-index <absolute-path-to-the-.dat-file>
```
3. Querying the index. In `build` folder, you should find another executable file called `query-index-fixed`. To solve the queries you should run:

```Bash
./query-index-fixed <absoulute-path-to-the-index-file> <absolute-path-to-the-query-file> <LIMIT>
```

Note that the second argument is the path to a file that contains all the queries. The queries of our benchmark are in directory `Queries`:

After running that command, you should see the number of the query, the number of results, and the elapsed time of each one of the queries with the following format:
```Bash
<query number>;<number of results>;<elapsed time>
```
---

At the moment, we can find the rest of the complementary material at [this webpage](http://compact-leapfrog.tk/). Note that we will find instructions to run the code there, and although the instructions are different from the ones in this repository, they should work too.
