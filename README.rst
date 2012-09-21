vavista.cache
=============

This module intends to provide an abstraction of the cache interface
for use with the VA VistA code. It should provide an identical interface
to the vavista.gtm module. 

This code is intended only for interfacing with VistA and is not 
a broad Pythonic interface to Cache.

Intersystems also provides a Python module called intersys.pythonbind.
This seems to map to Objects in the cache engine. I could not reconcile
this API with the VistA Globals usage.

At the moment, I only have trivial functionality working to verify the 
compiler.

::

    from vavista import cache

    cache.mset("^DD", "0")
    print cache.mget("^DD")

    cache.mset("^DD(0)", "1")
    print cache.mget("^DD(0)")


I can only execute code as user cacheusr due to permissions on the
cache IPCs.

::

    # ipcs  | grep cacheusr
    0x01020082 622610     cacheusr   600        342690     3                       
    0x00000000 32769      cacheusr   600        1         

I have hard-coded a cache path into the source. 
This is a not-well-documented requirement. Otherwise the code
could not connect to the server.

::

        CacheSetDir("/opt/cache/mgr");

