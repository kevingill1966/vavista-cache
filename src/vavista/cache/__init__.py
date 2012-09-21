import _cache as _mumps

#INOUT=_mumps.INOUT
#mexec=_mumps.mexec
#mget=_mumps.mget
#mset=_mumps.mset

def _cf_to_list(cf):
    # Convert the closed form value to a list.
    cf = str(cf)
    if cf.endswith(")"):
        parts = cf[:-1].split("(", 1)
        assert len(parts) == 2
        cf = [parts[0]] + parts[1].split(",")
    else:
        cf = [cf]
    for i, p in enumerate(cf):
        if p[0] == '"' and p[-1] == '"':
            cf[i] = p[1:-1]
        elif p[0] == "'" and p[-1] == "'":
            cf[i] = p[1:-1]
    if cf[0][0] == "^":
        cf[0] = cf[0][1:]
    return cf

def mset(cf, value):
    if type(value) == unicode:
        value = value.encode('utf-8', errors="ignore")
    else:
        value = str(value)
    gl = _cf_to_list(cf)
    return _mumps.mset(gl, value)

def mget(cf):
    gl = _cf_to_list(cf)
    return _mumps.mget(gl)

