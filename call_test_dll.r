# https://erpcoder.blog/2016/06/15/how-to-develop-a-c-dll-for-r-in-visual-studio-2015/

dyn.load("r_test_dll.dll")

inputa = sample((1:10) / 1.234, size = 5000000, replace=TRUE)

sum_output = 0;
sum_cpu_output = 0;

sum_output = .C("sum", as.integer(length(inputa)), as.double(inputa), result=as.double(sum_output))$result
sum_cpu_output = .C("sum_cpu", as.integer(length(inputa)), as.double(inputa), result=as.double(sum_cpu_output))$result

print(sum_output)
print(sum_cpu_output)
print(sum(inputa))
