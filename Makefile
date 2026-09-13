CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CPPFLAGS ?= -Iinclude
NVCC ?= nvcc
CUDAFLAGS ?= -std=c++17 -O2
CUDA_LIBS ?= -lcurand

BIN = monte_carlo_sim
SRC = src/main.cpp src/montecarlo.cpp src/black_scholes.cpp
CUDA_BIN = monte_carlo_cuda
CUDA_SRC = src/cuda_main.cpp src/cuda_montecarlo.cu

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@


run: all
	./$(BIN)

$(CUDA_BIN): $(CUDA_SRC)
	$(NVCC) $(CUDAFLAGS) $(CPPFLAGS) $^ $(CUDA_LIBS) -o $@

cuda: $(CUDA_BIN)

cuda-run: cuda
	./$(CUDA_BIN)

clean:
	rm -f $(BIN) $(CUDA_BIN)

.PHONY: all run cuda cuda-run clean
