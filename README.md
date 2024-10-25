# Compiler

A simple programming language compiler using llvm. 


### Usage
#### For linux : <br/>
Required
- llvm
- lld

```
./Compiler [srcLocation] [outputLocation]
```

### Building
#### For linux
Required : gtest, llvm, lld

```
cmake --build [buildDir] --target Compiler
```
```
cmake --build [buildDir] --target stdlinux
```


## Syntax

#### Sample Code

file 1
```
import "file2"

func int main(){
    test();
    return 0;
}
```

file2
```
func void test(){
    int i = 0;
    while(i < 3){
        printlnInt(i);
        i++;
    }
    return;
}
```



### IO Functions
- printlnInt(var)
- printlnChar(var)
- printInt(var)
- printChar(var)
- getNextInt()
- getNextChar()