export module maths;
//only the export functions are exposed publicly
export int add(int a, int b)
{
    return a + b;
}

export int multiply(int a, int b)
{
    return a * b;
}

int subtractInternal(int a, int b)
{
    return a - b;
}
