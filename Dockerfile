FROM gcc:13

WORKDIR /app

COPY server.cpp .

RUN g++ server.cpp -o server

CMD ["./server"]
