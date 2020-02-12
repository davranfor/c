# Script example

def app()
    name = "ast";
    version = 0.1;
end

def foo(a, b = 2)
    print("a = ", a, " b = ", b);
end
    
def main()
    print("Starting ...");

    foo(1);
    foo(3, 4);

    app.name = "yepa";
    print(app.name);

    print("Bye");
end

