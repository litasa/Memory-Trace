{
    "targets": [
        {
            "target_name": "NativeExtension",
            "sources": [ "NativeExtension.cc", "functions.cc", "ringbuffer.cc", "ringbuffer_wrapper.cc", "decoder.cc", "decoder_wrapper.cc" ],
            "include_dirs" : [
 	 			"<!(node -e \"require('nan')\")"
			]
        },
    ],
}