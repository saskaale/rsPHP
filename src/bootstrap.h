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

)";
