# https://erpcoder.blog/2016/06/15/how-to-develop-a-c-dll-for-r-in-visual-studio-2015/

dyn.load("r_test_dll.dll")

inputa = c(3, 5, 12, 14, 17, 18, 18, 20)
inputb = c(3, 5, 12, 14, 17, 18, 18, 20)
output = c(0, 0, 0, 0, 0, 0, 0, 0)

output = .C("sum", as.integer(length(inputa)), as.double(inputa), as.double(inputb), result=as.double(output))$result

print(output)