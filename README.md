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
4. Datasets and queries. The graph data and queries used in the paper can be found [here](https://zenodo.org/records/17438830?preview=1&token=eyJhbGciOiJIUzUxMiJ9.eyJpZCI6ImZiN2VmZWZkLTBhYzQtNDkxYy04NDllLWEwNWVmMThhMDQyYyIsImRhdGEiOnt9LCJyYW5kb20iOiJlMzM5OWUxYWE2OGNkNWE5OTYyZjVlMjY5OTYzMjc1ZCJ9.cNoMezQeNdKpn5BDRynV0kKdt0yL9cEdvZP0T4sIHFS1Z1Eb4rExoJ1MiauUKJi71Ldl4rZ0EbnA9PYfn3vWPQ)
---

