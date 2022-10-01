BrainFuck解释器

# 编译方法

gcc main.c brainfuck.c -o bf

# 使用方法

可以使用`./bf -?`查看帮助

`./bf -c <bf代码>`允许你在命令行调用执行bf代码

`./bf <bf代码文件>`运行bf代码文件

# 例子

在编译出bf可执行文件后，你可以试着执行`./bf bfcodes/helloworld.bf`或者`./bf bfcodes/a_add_b.bf`

bfcodes文件夹里就是brainfuck示例代码
