rm ../bin/eagle.app.v6.flash.bin
rm ../bin/eagle.app.v6.irom0text.bin
rm ../bin/eagle.app.v6.dump
rm ../bin/eagle.app.v6.S

cd .output\eagle\debug\image\

xt-objdump  -x -s eagle.app.v6.out > eagle.app.v6.dump
xcopy /y eagle.app.v6.dump ..\..\..\..\..\bin\

xt-objdump  -S eagle.app.v6.out > eagle.app.v6.S
xcopy /y eagle.app.v6.S ..\..\..\..\..\bin\

xt-objcopy -j .text eagle.app.v6.out eagle.app.v6.text.out
xt-objcopy -S -g -O binary eagle.app.v6.text.out eagle.app.v6.text.bin

xt-objcopy -j .data eagle.app.v6.out eagle.app.v6.data.out
xt-objcopy -S -g -O binary eagle.app.v6.data.out eagle.app.v6.data.bin

xt-objcopy -j .rodata eagle.app.v6.out eagle.app.v6.rodata.out
xt-objcopy -S -g -O binary eagle.app.v6.rodata.out eagle.app.v6.rodata.bin

xt-objcopy -j .irom0.text eagle.app.v6.out eagle.app.v6.irom0text.out
xt-objcopy -S -g -O binary eagle.app.v6.irom0text.out eagle.app.v6.irom0text.bin

xcopy /y eagle.app.v6.irom0text.bin ..\..\..\..\..\bin\

..\..\..\..\..\tools\gen_appbin.py eagle.app.v6.out v6

xcopy /y eagle.app.v6.flash.bin ..\..\..\..\..\bin\

cd ..\..\..\..\
