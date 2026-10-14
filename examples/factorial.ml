// Factorial of 6 and a small if/else.
let n = 6;
let result = 1;
while (n > 1) {
    result = result * n;
    n = n - 1;
}
print result;

if (result > 500) {
    print 1;
} else {
    print 0;
}
