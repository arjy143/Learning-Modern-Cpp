# Learning profiling tools like perf and valgrind

Perf required some setup.  
```sudo apt install linux-tools-generic linux-cloud-tools-generic```  


Need to make sure that the version of tools is correct, in /usr/bin/linux-tools/, and maybe symlink perf correctly.

Then I needed to update the sysctl.conf with:  
```echo "kernel.perf_level_paranoid=2" | sudo tee -a /etc/sysctl.conf```


Valgrind is as easy as ```apt install valgrind```
