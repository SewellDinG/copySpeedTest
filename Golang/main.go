package main

import (
    "crypto/md5"
    "crypto/rc4"
    "flag"
    "fmt"
    "log"
    "os"
    "time"
)

var (
    original  = flag.String("o", "", "Input Original File")
    target    = flag.String("t", "", "Input Target File")
    mode      = flag.Int("m", 1, "0 -> StreamCipherCrypto\n1 -> CopyBigFile")
    secretKey = flag.String("k", "", "Input Secret Key")
    bufSize   = flag.Int("n", 4096, "Input Buffer Size")
)

func timeCost(m int) func() {
    start := time.Now()
    return func() {
        tc := time.Since(start)
        mode := ""
        if m == 0 {
            mode = "Stream Cipher Crypto"
        } else if m == 1 {
            mode = "General Copy"
        }
        msf := fmt.Sprintf("Selected Mode: %s -->\tTime Cost: %v\n", mode, tc)
        writeToFile(msf)
    }
}

func writeToFile(msg string) {
    f, err := os.OpenFile("./timeCost.log", os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0644)
    if err != nil {
        log.Println(err.Error())
    }
    defer f.Close()
    _, err = f.Write([]byte(msg))
    if err != nil {
        log.Println(err.Error())
    }
}

func StreamCipherCrypto(original string, target string, secretKey string, bufSize int) error {
    fo, err := os.Open(original)
    if err != nil {
        fmt.Println("can't opened ", original)
        return err
    }
    defer fo.Close()
    ft, err := os.Create(target)
    if err != nil {
        fmt.Println("can't opened ", target)
        return err
    }
    defer ft.Close()
    md5sum := md5.Sum([]byte(secretKey))            //定义一个秘钥，将key生成MD5值就会得到一个16字节，即128位
    cipher, err := rc4.NewCipher([]byte(md5sum[:])) //定义一个加密器
    if err != nil {
        log.Fatal(err)
    }
    buf := make([]byte, bufSize) //定义一个指定大小的容器
    for {
        switch n, err := fo.Read(buf); true { //将标准输入的内容读取到buf中
        case n < 0:
            _, err = fmt.Fprintf(os.Stderr, "cat: error reading: %s\n", err.Error())
            return err
        case n == 0: //io.EOF，当读取到结尾时中止循环
            //break
            return nil
        case n > 0:
            dst := buf[:n]                //将读取到的内容取出来
            cipher.XORKeyStream(dst, dst) //进行原地加密
            ft.Write(dst)                 //将加密后的数据标准输出
        }
    }
    return nil
}

func CopyBigFile(original string, target string, bufSize int) error {
    fo, err := os.Open(original)
    if err != nil {
        fmt.Println("can't opened ", original)
        return err
    }
    defer fo.Close()
    ft, err := os.Create(target)
    if err != nil {
        fmt.Println("can't opened ", target)
        return err
    }
    defer ft.Close()
    buf := make([]byte, bufSize)
    for {
        switch n, err := fo.Read(buf); true {
        case n < 0:
            _, err = fmt.Fprintf(os.Stderr, "cat: error reading: %s\n", err.Error())
            return err
        case n == 0:
            return nil
        case n > 0:
            dst := buf[:n]
            ft.Write(dst)
        }
    }
    return nil
}

func main() {
    flag.Parse()
    m := *mode
    var err error
    defer timeCost(m)() //耗时计算
    if m == 0 {
        err = StreamCipherCrypto(*original, *target, *secretKey, *bufSize)
    } else if m == 1 {
        err = CopyBigFile(*original, *target, *bufSize)
    }
    if err != nil {
        os.Exit(1)
    }
}
