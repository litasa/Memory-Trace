{
    "targets": [
        {
            "target_name": "Encryption",
            "sources": [ "./cpp/NativeExtension.cc", "./cpp/ringbuffer.cc", "./cpp/decoder.cc", "./cpp/decoder_wrapper.cc" , "./cpp/memory_state.cc"],
            "include_dirs" : [
 	 			"<!(node -e \"require('nan')\")"
			]
        },
    ],
}