#define RSPHP_BOOTSTRAP rsphp_bootstrap
static const char *rsphp_bootstrap = R"(

function min(a, b)
{
    if (a < b) return a;
    return b;
}

function max(a, b)
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
    if (v === false) {
        print "Assertion " + c + " failed!";
        exit(1);
    }
}

function forEach(&a, f)
{
    c = count(a);
    for (i = 0; i < c; ++i) {
        f(a[i]);
    }
}

function indexOf(&a, v)
{
    c = count(a);
    for (i = 0; i < c; ++i) {
        if (a[i] == v) {
            return i;
        }
    }
    return -1;
}

)";
