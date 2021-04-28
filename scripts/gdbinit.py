import gdb

def main():
    gdb.execute('target remote localhost:3008')
    gdb.execute('b main')
    gdb.execute('c')

if __name__ == "__main__":
    main()