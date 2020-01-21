print("Starting ...", "\n");

# for 1 to 999 print multiples of 100
for (i = 1, i < 1e3, i += 1)
    if ((i % 100) == 0)
        println("i = ", i);
    end
end

test = 1;
if (test == 0)
    println("test == 0");
elif (test > 1)
    println("test > 1");
else
    println("test == 1");
    if (0)
        0;
    else
        println("test == 1");
        println("test == 1");
    end
end

println((3*2)+(3*2));
println(pow(trunc((3 + 0.14)), 2));
println("Bye");
3+2;
