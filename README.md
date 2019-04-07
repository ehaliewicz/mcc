
# mcc - A Mini C-like Compiler

Compiler for a very small c-like language.

## syntax
Supports simplified c-like syntax (with a few deviations) for function declarations, variable declarations and assignments, if statements, while loops, and recursive function calls.
```
int fib(int cnt) {
    if(cnt < 2) {
        return cnt;
    } else {
        return fib(cnt-2) + fib(cnt-1);
    }
}

int nl = 10;
int i = read();
int a = fib(i);

print(a);
putc(nl);


int fib_iter(int cnt) {
    int a = 0;
    int b = 1;
    while(cnt > 0) {
        int c = a+b;
	a <- b;
        b <- c;
        cnt <- cnt-1;
    }
    return a;
}

a <- fib_iter(i);
print(a);
putc(nl);
```

## usage
#### options
```
 -e/---execute      ; executed compiled bytecode immediately
 -d/---disassemble  ; show disassembled bytecode
 ; providing no option will immediately print compiled bytecode (in binary) to the terminal,
 ; this is mostly for debugging purposes
```
#### examples
```
gcc mcc.c -o mcc
./mcc fib -e    
> 12   
144
./mcc fib -e  
> 13
233
./mcc guess_a_number -e
> 0
too low
> 20
too high
> 15
too high
> 12
  
./mcc fib -d
---- compiled output: 65 bytes ----
;;; this is just an excerpt of the whole program
0x000c: PUSHENV 0x0000/0
0x000e: LIT 0x0000/0
0x0010: CMP
0x0011: BNE 0x0007/7
0x0013: PUSHENV 0x0001/1
0x0015: RET
0x0016: BRA 0x0011/17
0x0018: PUSHENV 0x0000/0
0x001a: LIT 0x0001/1
0x001c: SUB
0x001d: PUSHENV 0x0002/2
0x001f: PUSHENV 0x0001/1
0x0021: PUSHENV 0x0002/2
0x0023: ADD
0x0024: CALL 0x0004/4
```