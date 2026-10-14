// Prints 1..15, but prints 0 for multiples of 3 (uses a - (a/3)*3 as modulo).
let i = 1;
while (i <= 15) {
    let r = i - (i / 3) * 3;
    if (r == 0) {
        print 0;
    } else {
        print i;
    }
    i = i + 1;
}
