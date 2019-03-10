# 编译原理实验二设计报告

## 小组成员

祝冠琪、王之玺、王海林

## if-else、break、continue的设计

### **设计人员：祝冠琪、王之玺**

#### if-else的实现

对于if-else的实现，我们首先参考了**ANSI C**的语法设计

```
selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;
```

所以我们的**if-else**实现如下

```
else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_ELSE, SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys, breaklist, continuelist);
		if (sym == SYM_ELSE)//else part
		{
			cx2 = cx;
			gen(JMP, 0, 0);
			code[cx1].a = cx;
			getsym();
			statement(fsys, breaklist, continuelist);
			code[cx2].a = cx;
		}
		else{
			code[cx1].a = cx;
		}
	}
```

可以看到前19行就相当于是`  IF '(' expression ')' statement`的部分。从第20行开始就是我们实现的**else**部分。首先如果进入到**else**，那么要先生成一个JMP指令，因为**if-else**代码的样式应该是这样

```
if expression
JPC L1

expression 为真的所运行的内容
JMP END

L1：
else if expression
JPC L2

expression 为真的所运行的内容
JMP END

L2：
else

else部分运行的内容

END:
if-else外的语句内容
```

所以当要进入**else**部分的时候，我们需要先生成**JMP**指令，当上一个块为真时运行的代码运行完了，需要跳到**if-else**外，然后再运行`code[cx1].a = cx;`这一句话是对上一部分**JPC**指令的回填，也就是跳转到**else**后要运行的语句。这里我们需要用到回填技术，首先`cx2 = cx;`这个是让**cx2**记录**JMP**指令的索引，这样当运行完**else**部分的**statement**后`code[cx2].a = cx`就可以让**JMP**指令跳出**if-else**外。最后假如**sym！=SYM_ELSE**那么我们直接`code[cx1].a = cx;`即可，和原来**if**的实现是一样的。

#### break、continue的实现

对于**break、continue**我们首先引入了一个全局变量`int count = 0`这个变量是记录while循环嵌套的层次，当没有进入while循环时，层次为0。进入一层while循环，层次就加一。

其次是需要介绍**breaklist与continuelist**。先介绍**breaklist**。**breaklist**是一个32*32的二维数组，我们首先定义了一个宏`#define MAXDEPTH   32`这个表示的是while循环可嵌套的最大深度。所以**breaklist**的意思就是，最多可以有32层循环且每层循环里面最多有32个break。

然后是**continuelist**。**continuelist**是一个int[32]的一维数组，至于为什么不是二维数组等代码具体实现的时候再讲。他代表的是每一层while循环里面的continue需要跳转的地址。

接下来是我们的代码实现

**while部分**

```
else if (sym == SYM_WHILE)
	{ // while statement
		count++;
		if(breaklist == NULL && continuelist == NULL){
			breaklist = (int**)malloc(sizeof(int*) * MAXDEPTH);
			for(size_t i = 0; i < MAXDEPTH; i++)
			{
				breaklist[i] = (int*)malloc(sizeof(int)*MAXDEPTH);
			}
			continuelist = (int*)malloc(sizeof(int) * MAXDEPTH);
			for(size_t i = 0; i < MAXDEPTH; i++)
			{
				continuelist[i] = 0;
			}
			
			for(size_t i = 0; i < MAXDEPTH; i++)
			{
				
				for(size_t j = 0; j < MAXDEPTH; j++)
				{
					breaklist[i][j] = 0;
				}	
			}			
		}//初始化
		cx1 = cx;
		int i;
		continuelist[count] = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		set1 = createset(SYM_CONTINUE, SYM_BREAK, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set, breaklist, continuelist);
		destroyset(set1);
		destroyset(set);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
		for(i = 0; breaklist[count][i] != 0; i++){
		   //if break exists
			code[breaklist[count][i]].a = cx;
			breaklist[count][i] = 0;
		}
		count--;
	}
```

可以看到在代码的第三行，每一次进入while我们都要进行`count++`，然后我们需要知道

```
void statement(symset fsys, int **breaklist, int *continuelist)
```

这是**while**所在函数**statement**的声明，所以当传进来的**breaklist**和**continuelist**都是**null**的时候，说明第一次进入**while**。要进行两者的初始化。

```
if(breaklist == NULL && continuelist == NULL){
			breaklist = (int**)malloc(sizeof(int*) * MAXDEPTH);
			for(size_t i = 0; i < MAXDEPTH; i++)
			{
				breaklist[i] = (int*)malloc(sizeof(int)*MAXDEPTH);
			}
			continuelist = (int*)malloc(sizeof(int) * MAXDEPTH);
			for(size_t i = 0; i < MAXDEPTH; i++)
			{
				continuelist[i] = 0;
			}
			
			for(size_t i = 0; i < MAXDEPTH; i++)
			{
				
				for(size_t j = 0; j < MAXDEPTH; j++)
				{
					breaklist[i][j] = 0;
				}
				
			}
}
```

这个就是初始化的部分，把两者都初始化为0。

**while循环的运行样式**

```
L1:
while expression
JPC END

expression 为真的所运行的内容
...
遇到continue
JMP L1
...
遇到break
JMP END
...
JMP L1

END:
while外的语句内容
```



然后在代码的第27行，`continuelist[count] = cx`这个是记录当前层**while**中**continue**需要**JMP**的地址即上述样式中的**L1**。之后会进入到**while**里面的**statement**（需要注意的是，我们会将**SYM_CONTINUE**和**SYM_BREAK**放入**statement**的**set**中）。若在里面遇到**continue**则会运行以下语句

```
else if (sym == SYM_CONTINUE)
	{
		int sym1 = sym;
		test(fsys, phi, 19);
		if (sym1 == sym) {
			gen(JMP, 0, continuelist[count]);
			getsym();
		}
		else
		{
			return;
		}
	}
```

这里的第三行第四行是检验**continue**是否在**while**循环内，若在**while**循环外出现了**continue**就报错。若在**while**内，那么**test**后的**sym**应该是不变的，这时候我们就产生**JMP**指令`gen(JMP, 0, continuelist[count])`因为之前已经说过**continuelist[count]**内放的是这一层**continue**需要跳转的地址，并且同一层**continue**需要跳转的地址应该是一样的，这就是**continuelist**是一维数组的原因。

如若在里面遇到**break**运行以下语句

```
else if (sym == SYM_BREAK)
	{
		int sym1 = sym;
		test(fsys, phi, 19);
		if (sym1 == sym) {
			int i;
			for (i = 0; breaklist[count][i] != 0; i++);
			breaklist[count][i] = cx;
			gen(JMP, 0, 0);
			getsym();
		}
		else
		{
			return;
		}
	}
```



3、4行同**continue**，是用来检错的。对于第七行我们需要知道，一个**while**里面不止一个**break**，并且每个**break**都是需要回填的，所以我们先找到了**breaklist**中第一个空的位置`for (i = 0; breaklist[count][i] != 0; i++);`然后在这个位置记录下当前**break**的**cx**，用作以后的回填，接着产生一条`gen(JMP, 0, 0);`用作以后的回填。

当处理完**while**内部的**statement**后我们需要产生一条**JMP**指令跳转到**while**开始部分，接下来回填一开始**while**条件判断的**JPC**指令，随后我们需要回填**break**的**JMP**指令

```
for(i = 0; breaklist[count][i] != 0; i++){
		   //if break exists
			code[breaklist[count][i]].a = cx;
			breaklist[count][i] = 0;
		}
```

最后我们需要`count--`退出这一层while。

## 连续赋值的设计

### **设计人员：祝冠琪、王之玺、王海林**

### 最初的设计

我们用过两种方式实现连续赋值。最开始，我们修改了文法结构，如下：

```
Statement：id := expression
Expression: id B
         	| numfunc
B: := expression
  |OPS
  |ε
OPS: + Term
   | - Term
   | * Factor
   | / Factor
Numfunc: T OP1
OP1:  +  Term  OP1
   |  -  Term  OP1
   |  ε
T:  F  OP2
OP2： *  F  OP2
   |  /  F  OP2
   |  ε
F:  number
 | - F
 | (expression)

```

通过修改表达式的文法，使他能产生赋值表达式。为了使新的语法不产生不符合连续赋值要求的式子（算术表达式后跟赋值号），我们将读到第一个符号位id或者数字分成两种产生式。其中numfunc代表不是id开头的表达式，B代表赋值号开头的式子或者算数符号开头的式子，T代表数字开头的项，F代表数字开头的因子。

但是可以看到这一种设计有一个很大的缺点，太过于复杂并且不怎么自然。

### 受老师启发后的设计

在听过老师的设计思路后我们跟换了实现方式，如下：

```
Statement: assign_exp (| if | while ...等不属于连续赋值的部分)
Assign_exp: expression A
A: := assign_exp A
   | ε

```

这种实现思路简洁有效，在添加最少代码的情况下能达到要求。将statement中整个赋值表达式由assign_exp产生，assign_exp由expression读取赋值的左值并读取连续赋值的最右边的右值表达式。由于expression在读取到单个id时会将他的值置于栈顶，所以需要检测到这种情况并删除产生的LOD指令。

```
void statement(symset fsys, int **breaklist, int *continuelist)
{
	int i, cx1, cx2;
	symset set1, set;

	//test(fsys, phi, 19);
	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		assign_exp(fsys);
		cx--;      //pop the last LOD instruction
	}
}

void assign_exp(symset fsys)
{
    mask* mk;
    int i;
	i = position(id);
    expression(fsys);
    if(sym == SYM_BECOMES){
        if(code[cx-1].f != LOD){
            error(19);
        }
        cx--;
        mk = (mask*) &table[i];
        getsym();
        assign_exp(fsys);
        gen(STO, level - mk->level, mk->address);
        gen(LOD, level - mk->level, mk->address);
    }
    else{
        return;
    }
}


//对于factor读到左括号时的更改
else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			assign_exp(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
```

在statement中，如果当前符号是id，则进入assign_exp并在最后删除多余的LOD指令（因为在assign_exp中每一个赋值号都会产生STO与LOD，但是对于最左边的赋值号的左值显然不需要LOD）。在assign_exp中首先用expression读取赋值号之前的所有符号，在下一个符号是赋值符号时，如果expression不是单个ident，则expression产生的最后一条指令一定不是LOD，所以可以凭此报错。如果正确，则删除多余的LOD指令并调用assign_exp继续处理剩下的连续赋值语句。每个assign_exp会将该赋值表达式的值置于栈顶，所以在结尾添加STO指令可以将右边赋值表达式的值赋给左值。

## 实验结果

### 测试代码1

```
//测试代码
var i,j,k;
begin
  i := 0;
  j := 0;
  while i < 10 do
  begin
    while j < 10 do
    begin
        if i = 5 then
        begin
            k := 15;
            break;
        end
        else if i = 7 then
        begin
            k := 17;
            break;
        end;
        j := j + 1;
    end;
    j := 0;
    if i = 5 then break
    else 
    begin
      i := i + 1;
      k := 20;
    end;
  end;
end.
```

```
//运行结果
Begin executing PL/0 program.
0
0
1
2
3
4
5
6
7
8
9
10
0
1
20
1
2
3
4
5
6
7
8
9
10
0
2
20
1
2
3
4
5
6
7
8
9
10
0
3
20
1
2
3
4
5
6
7
8
9
10
0
4
20
1
2
3
4
5
6
7
8
9
10
0
5
20
15
0
End executing PL/0 program.

    0 JMP       0       1
    1 INT       0       6
    2 LIT       0       0
    3 STO       0       3
    4 LIT       0       0
    5 STO       0       4
    6 LOD       0       3
    7 LIT       0       10
    8 OPR       0       9
    9 JPC       0       49
   10 LOD       0       4
   11 LIT       0       10
   12 OPR       0       9
   13 JPC       0       34
   14 LOD       0       3
   15 LIT       0       5
   16 OPR       0       7
   17 JPC       0       22
   18 LIT       0       15
   19 STO       0       5
   20 JMP       0       34
   21 JMP       0       29
   22 LOD       0       3
   23 LIT       0       7
   24 OPR       0       7
   25 JPC       0       29
   26 LIT       0       17
   27 STO       0       5
   28 JMP       0       34
   29 LOD       0       4
   30 LIT       0       1
   31 OPR       0       2
   32 STO       0       4
   33 JMP       0       10
   34 LIT       0       0
   35 STO       0       4
   36 LOD       0       3
   37 LIT       0       5
   38 OPR       0       7
   39 JPC       0       42
   40 JMP       0       49
   41 JMP       0       48
   42 LOD       0       3
   43 LIT       0       1
   44 OPR       0       2
   45 STO       0       3
   46 LIT       0       20
   47 STO       0       5
   48 JMP       0       6
   49 OPR       0       0
```

```
#以下是等价的python程序的运行结果，可以看到是一样的
0                             
0                             
1                             
2                             
3                             
4                             
5                             
6                             
7                             
8                             
9                             
10                            
0                             
1                             
20                            
1                             
2                             
3                             
4                             
5                             
6                             
7                             
8                             
9                             
10                            
0                             
2                             
20                            
1                             
2                             
3                             
4                             
5                             
6                             
7                             
8                             
9                             
10                            
0                             
3                             
20                            
1                             
2                             
3                             
4                             
5                             
6                             
7                             
8                             
9                             
10                            
0                             
4                             
20                            
1                             
2                             
3                             
4                             
5                             
6                             
7                             
8                             
9                             
10                            
0                             
5                             
20                            
15                            
0                             
```

### 测试代码2

```
var i,j,k;
begin
  i := 0;
  j := 0;
  while i < 10 do
  begin
    while j < 10 do
    begin
        if i = 5 then
        begin
            k := 15;
            j := j + 1;
            continue;
        end
        else if i = 7 then
        begin
            k := 17;
            j := j + 1;
            continue;
        end;
        j := j + 1;
    end;
    j := 0;
    if i = 5 then begin
      i := i + 1;
      continue;
    end
    else 
    begin
      i := i + 1;
      k := 20;
    end;
  end;
end.
```

```
结果：
Begin executing PL/0 program.
0
0
1
2
3
4
5
6
7
8
9
10
0
1
20
1
2
3
4
5
6
7
8
9
10
0
2
20
1
2
3
4
5
6
7
8
9
10
0
3
20
1
2
3
4
5
6
7
8
9
10
0
4
20
1
2
3
4
5
6
7
8
9
10
0
5
20
15
1
15
2
15
3
15
4
15
5
15
6
15
7
15
8
15
9
15
10
0
6
1
2
3
4
5
6
7
8
9
10
0
7
20
17
1
17
2
17
3
17
4
17
5
17
6
17
7
17
8
17
9
17
10
0
8
20
1
2
3
4
5
6
7
8
9
10
0
9
20
1
2
3
4
5
6
7
8
9
10
0
10
20
End executing PL/0 program.

    0 JMP       0       1
    1 INT       0       6
    2 LIT       0       0
    3 STO       0       3
    4 LIT       0       0
    5 STO       0       4
    6 LOD       0       3
    7 LIT       0       10
    8 OPR       0       9
    9 JPC       0       61
   10 LOD       0       4
   11 LIT       0       10
   12 OPR       0       9
   13 JPC       0       42
   14 LOD       0       3
   15 LIT       0       5
   16 OPR       0       7
   17 JPC       0       26
   18 LIT       0       15
   19 STO       0       5
   20 LOD       0       4
   21 LIT       0       1
   22 OPR       0       2
   23 STO       0       4
   24 JMP       0       10
   25 JMP       0       37
   26 LOD       0       3
   27 LIT       0       7
   28 OPR       0       7
   29 JPC       0       37
   30 LIT       0       17
   31 STO       0       5
   32 LOD       0       4
   33 LIT       0       1
   34 OPR       0       2
   35 STO       0       4
   36 JMP       0       10
   37 LOD       0       4
   38 LIT       0       1
   39 OPR       0       2
   40 STO       0       4
   41 JMP       0       10
   42 LIT       0       0
   43 STO       0       4
   44 LOD       0       3
   45 LIT       0       5
   46 OPR       0       7
   47 JPC       0       54
   48 LOD       0       3
   49 LIT       0       1
   50 OPR       0       2
   51 STO       0       3
   52 JMP       0       6
   53 JMP       0       60
   54 LOD       0       3
   55 LIT       0       1
   56 OPR       0       2
   57 STO       0       3
   58 LIT       0       20
   59 STO       0       5
   60 JMP       0       6
   61 OPR       0       0
```

```
用等价python程序跑的结果，结果是一样的：
0   
0   
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
1   
20  
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
2   
20  
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
3   
20  
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
4   
20  
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
5   
20  
15  
1   
15  
2   
15  
3   
15  
4   
15  
5   
15  
6   
15  
7   
15  
8   
15  
9   
15  
10  
0   
6   
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
7   
20  
17  
1   
17  
2   
17  
3   
17  
4   
17  
5   
17  
6   
17  
7   
17  
8   
17  
9   
17  
10  
0   
8   
20  
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
9   
20  
1   
2   
3   
4   
5   
6   
7   
8   
9   
10  
0   
10  
20  
```

### 测试代码3

```
    0  var i,j,k,l;
    1  begin
    2   i := j := k := (l := 100);
   10   if i > 50 then k := 60
   15      else k:=100;
   19  end.

    1 INT       0       7
    2 LIT       0       100
    3 STO       0       6
    4 LOD       0       6
    5 STO       0       5
    6 LOD       0       5
    7 STO       0       4
    8 LOD       0       4
    9 STO       0       3
   10 LOD       0       3
   11 LIT       0       50
   12 OPR       0       11
   13 JPC       0       17
   14 LIT       0       60
   15 STO       0       5
   16 JMP       0       19
   17 LIT       0       100
   18 STO       0       5
   19 OPR       0       0

Begin executing PL/0 program.
100
100
100
100
60
End executing PL/0 program.

    0 JMP       0       1
    1 INT       0       7
    2 LIT       0       100
    3 STO       0       6
    4 LOD       0       6
    5 STO       0       5
    6 LOD       0       5
    7 STO       0       4
    8 LOD       0       4
    9 STO       0       3
   10 LOD       0       3
   11 LIT       0       50
   12 OPR       0       11
   13 JPC       0       17
   14 LIT       0       60
   15 STO       0       5
   16 JMP       0       19
   17 LIT       0       100
   18 STO       0       5
   19 OPR       0       0
```



## 实验体会

这一次实验让我们对梯度下降和LL（1）有了更加深刻的认识，并且对编译器是如何工作的，有了更加深入的了解。在设计文法以达到我们期望的功能时，如果想通过文法本身的约束来识别错误的输入有时需要付出较大的代价（尤其在连续赋值的实现中）。此时，我们可以考虑将检测错误的功能由后续翻译部分来实现。例如，如果想检测赋值号左边的表达式是否为合法左值，如果要使用文法进行限制，我们需要额外增加许多非终结符以保证非法左值后续不能产生赋值号。但是，如果我们在文法设计上放松要求，允许一个非法左值的表达式后续跟随赋值号，而检错由翻译后产生的指令来判断，则可以复用之前的许多函数从而简化程序。