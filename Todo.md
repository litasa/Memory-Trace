Todo:

# Ideas
* Let heap_id 0 be reserved for systems that does not care about multiple heaps

# Fixes
* better communication between visualizer and source.
* A forced allocation should register for two blocks.
* When removing core also check for forced allocations

# Features
* Create memory internal view
* Overlay two different runs ontop of eachother

# UI
* Add view for number of allocations per time
* Filter allocations with regards to Y value also

# Other
* (experiment) Create a fast and good database solution to stream from
* (experiment) read x amount of operations and then pause

# For Stingray
* Use the allocator registry to indicate which allocators should be shown in the visualizer

# Done
* ~~Fix network error at shutdown.~~
* ~~Update visualization scales should not randomly snap to edge~~
* ~~add slider(or number box) for how much to filter~~
* ~~Add a timesampler (for example get the current status of the heap every 200 ms)~~
* ~~Add the ability to only track one specific heap~~
* ~~Remove ugly std::cout code~~
