{
    "targets": [
        {
            "target_name": "Encryption",
            "sources": [ "./cpp/NativeExtension.cc", "./cpp/ringbuffer.cc", "./cpp/decoder.cc", "./cpp/decoder_wrapper.cc" ,
                         "./cpp/memory_state.cc", "./cpp/core.cc", "./cpp/heap.cc", "./cpp/memory_object.h",
                         "./cpp/sqlite3/sqlite3.c", "./cpp/sqlite3/shell.c"],
            "include_dirs" : [
 	 			"<!(node -e \"require('nan')\")"
			]
        },
    ],
}