- Fixed `bootutil_img_hash()` ignoring the result of the SHA calls. A
  failure to initialise, update, finalise or release the hash context
  now fails the hash instead of passing a partial or stale digest on to
  signature verification.
