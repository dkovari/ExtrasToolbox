%test mexIngeterface
clc
p = mexInterfaceTest('new');

'press key to call fn'
pause
mexInterfaceTest('fn',p)

'press key to delete'
pause
mexInterfaceTest('delete',p);

clear mex;
