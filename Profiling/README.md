# Learning profiling tools like perf and valgrind

Perf required some setup.  
```sudo apt install linux-tools-generic linux-cloud-tools-generic```  


Need to make sure that the version of tools is correct, in /usr/bin/linux-tools/, and maybe symlink perf correctly.

Then I needed to update the sysctl.conf with:  
```echo "kernel.perf_level_paranoid=2" | sudo tee -a /etc/sysctl.conf```


Valgrind is as easy as ```apt install valgrind```. 

then compile something with -g and whatever optimisation flag you want, then use ```valgrind ./your_executable``` to see if theres any memory leaks. your programs will run many times slower but will detect any issues

Tools:
- memcheck for checking memory leaks
- helgrind for data race detection in mulithreaded programs
- cachegrind for simulating cpu cache in order to optimise better
- massif for heap memory usage over time
- callgrind - for profiling cpu graph performance

If you want to specify memcheck you can use the flag ```--leak-check=yes```.