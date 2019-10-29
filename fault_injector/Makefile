INJECTORS=fault_injector.so fork_failed.so 2socket_crash.so

all: fault_injector.so

fault_injector.so: fault_injector.cc 
	g++ -m32 $< -ldl -lrt -shared -o $@

clean:
	rm -f fault_injector.so
