How to run: 

- First make sure that you have boost_build/ folder in the same folder as the code files. This is the folder that stores all the Boost libraries needed for multithreading.

- From the command line, run

	$ make

- Generate random content files:

	$./util

- Run router controller:

	$./routercontroller

- Run each host (individually)

	$./host

- Copy the content files into each host's initial storage at hostX/ folder, where X is the ID of the host. 

- At each host terminal, enter the ID of the content that this host wants to retain, then wait for that content to delivered to the host. 

- The delivered content is saved in folder: storeX/, where X is the ID of the host. 
