# zeroserv (Øserv)

[![Join the chat at https://gitter.im/zunc/zeroserv](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/zunc/zeroserv?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

* Nền tảng server dựa trên kiến trúc HAProxy (http://www.haproxy.org)
* **lightweight** viết trên ansi C, nhỏ gọn, giản dị và dễ hiểu
* **event-driven** điều khiển asynchronous socket với event-driven, nhẹ nhàng êm ái và vẫn đảm bảo hiệu suất
* **performance** lockfree, tối ưu hóa trên CPU nhiều core
* Loại bỏ vấn đề content-switch với thread, bằng tư tưởng: Một luồng chấp hết ("One thread to rule them all") 

## Một nền tảng động
* Cho ứng dụng được phát triển dễ dàng như một handler với một interface hỗ trợ sẵn
* Hỗ trợ cả UDP lẫn TCP
* Các handler hiện tại đã phát triển
```
echo (TCP/UDP) : simple echo
http : simple http
memcached (TCP)
pubsub: hỗ trợ cả plain và http protocol
```

## Nên dùng khi nào ?
* Øserv đặc biệt hiệu quả trong trường hợp mỗi kết nối lên server dùng một tài nguyên độc lập như xây dựng proxy, socks ...

## Hiệu suất
* Một thử nghiệm nhỏ về hiệu suất với **apachebench**, với tùy chọn connection không keep-alive, trả về giá trị đơn giản
```
zeroserv : trả về uri request
nginx : trả về trang index trống
```
|zeroserv      | nginx        |
|--------------|--------------|
|32615.28 req/s|29687.42 req/s|


## Dễ dùng
* Viết một handler đơn giản và sử dụng chỉ với một dòng code
```
tcp_create_listener("echo", 8080, &handler_echo);
```
