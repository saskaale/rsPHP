#define RSPHP_BOOTSTRAP rsphp_bootstrap
static const char *rsphp_bootstrap = R"(

function min(const &a, const &b)
{
    if (a < b) return a;
    return b;
}

function max(const &a, const &b)
{
    if (a > b) return a;
    return b;
}

function swap(&a, &b)
{
    tmp = a;
    a = b;
    b = tmp;
}

function assert(c, v)
{
    if (v === undefined) {
        v = c;
        c = "";
    }
    if (!v) {
        if (c != "")
            print "Assertion '" + c + "' failed!";
        else
            print "Assertion failed!";
        exit(1);
    }
}

function push(&a, v)
{
    __push_internal(a, v);
}

function copy(a)
{
    return a;
}

function merge(const &a, const &b)
{
    s = Array(count(a) + count(b));
    p = function(v) {
        s.push(v);
    };
    a.forEach(p);
    b.forEach(p);
    return s;
}

function forEach(const &a, f)
{
    c = a.count();
    for (i = 0; i < c; ++i) {
        if (f(a[i], i) !== undefined) {
            break;
        }
    }
}

function indexOf(const &a, v)
{
    o = -1;
    a.forEach(function(s, i) {
        if (s == v) {
            o = i;
            return true;
        }
    });
    return o;
}

)";
