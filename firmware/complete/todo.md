bugs:
* temperature readings off for temp1 - looks mostly okay for temp2 now
* amp readings off (amp setting grob richtig) - reading off weil 2xtransistoren defekt denk ich
* volt reading not stable at last digit
* limited state handling kaputt
* steuerung nicht möglich - ging noch in e03df41b7a4c49042a71132f5b700ccbe3eb6b6c

missing features:
* lock function
* memory function
* remote function
* graph with all curves
* standby mit aus signal statt dac

hardware:
* maybe caps?? die restlichen scheinen aber okay

workaround found for:
* inkrementalgeber gibt bei schnellem drehen manchmal/selten fake signale
