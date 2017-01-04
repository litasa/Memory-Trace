{
    "targets": [
        {
            "target_name": "Encryption",
            "sources": [ "NativeExtension.cc", "ringbuffer.cc", "decoder.cc", "decoder_wrapper.cc" , "memory_state.cc"],
            "include_dirs" : [
 	 			"<!(node -e \"require('nan')\")"
			]
        },
    ],
}