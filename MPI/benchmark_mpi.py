"""
Script để đo hiệu suất MPI Adaptive Huffman Coding
Chạy tất cả test files với các số lượng processes khác nhau và lưu kết quả vào CSV
"""

import subprocess
import os
import re
import csv

def compile_mpi():
    """Compile MPI code"""
    print("Compiling MPI code...")
    
    try:
        # Sử dụng command string với shell=True để xử lý paths với spaces
        cmd = 'g++ -I"C:\\Program Files (x86)\\Microsoft SDKs\\MPI\\Include" -fopenmp mpi_adaptive_huffman.cpp -L"C:\\Program Files (x86)\\Microsoft SDKs\\MPI\\Lib\\x64" -lmsmpi -o mpi_huffman.exe'
        
        result = subprocess.run(cmd, capture_output=True, text=True, check=True, shell=True)
        print("Compilation successful")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e.stderr}")
        print(f"Command used: {cmd}")
        return False

def setup_mpi_environment():
    """Setup MPI environment PATH"""
    print("Setting up MPI environment...")
    
    # Add MPI to PATH
    mpi_path = "C:\\Program Files\\Microsoft MPI\\Bin"
    current_path = os.environ.get('PATH', '')
    
    if mpi_path not in current_path:
        os.environ['PATH'] = current_path + ";" + mpi_path
        print(f"Added MPI to PATH: {mpi_path}")
    else:
        print("MPI already in PATH")

def run_mpi_test(test_file, processes):
    """Chạy test với file cụ thể và số processes, extract thời gian encoding, decoding và compression ratio"""
    print(f"Running test: {test_file} with {processes} processes")
    
    try:
        # Chạy mpiexec với test file
        result = subprocess.run([
            "mpiexec", "-n", str(processes), ".\\mpi_huffman.exe", test_file
        ], capture_output=True, text=True, check=True, timeout=300, shell=True)  # 5 minutes timeout
        
        # Extract thông tin từ output
        output = result.stdout
        
        # Tìm các thông tin cần thiết
        encoder_match = re.search(r"Encoder Time:\s+([\d\.]+)\s+seconds", output)
        decoder_match = re.search(r"Decoder Time:\s+([\d\.]+)\s+seconds", output)
        compression_match = re.search(r"Compression Ratio:\s+([\d\.]+)", output)
        
        if encoder_match and decoder_match and compression_match:
            encoder_time = float(encoder_match.group(1))
            decoder_time = float(decoder_match.group(1))
            compression_ratio = float(compression_match.group(1))
            
            print(f"  Encoder Time: {encoder_time}s, Decoder Time: {decoder_time}s, Compression Ratio: {compression_ratio}")
            return {
                'encoder_time': encoder_time,
                'decoder_time': decoder_time,
                'compression_ratio': compression_ratio
            }
        else:
            print(f"Could not parse all required data from output")
            print(f"Output: {output[:200]}...")
            return None
            
    except subprocess.TimeoutExpired:
        print(f"Test timed out after 5 minutes")
        return None
    except subprocess.CalledProcessError as e:
        print(f"Test failed: {e.stderr}")
        return None

def main():
    """Main function"""    
    # Get current directory (should be MPI directory)
    project_dir = os.getcwd()
    print(f"Working in: {project_dir}")
    
    # Setup MPI environment
    setup_mpi_environment()
    
    # Compile MPI code
    if not compile_mpi():
        return
    
    # Test files
    test_files = ["10", "100", "1000", "10000", "100000", "1000000", "10000000", "shakespeare_ascii"]
    
    # Số processes để test (1, 2, 4, 8 - phù hợp với số logical cores)
    processes_list = [1, 2, 4, 8]
    
    # Kết quả
    results = []
    
    print("\nRunning Tests")
    
    for test_file in test_files:
        for processes in processes_list:
            print(f"\n--- Test {test_file}.txt with {processes} processes ---")
            
            # Chạy test 5 lần và lấy trung bình
            encoder_times = []
            decoder_times = []
            compression_ratios = []
            
            for run in range(5):
                print(f"  Run {run + 1}/5...")
                test_result = run_mpi_test(test_file, processes)
                
                if test_result is not None:
                    encoder_times.append(test_result['encoder_time'])
                    decoder_times.append(test_result['decoder_time'])
                    compression_ratios.append(test_result['compression_ratio'])
                else:
                    print(f"  Failed on run {run + 1}")
                    break
            
            if encoder_times:
                avg_encoder_time = sum(encoder_times) / len(encoder_times)
                avg_decoder_time = sum(decoder_times) / len(decoder_times)
                avg_compression_ratio = sum(compression_ratios) / len(compression_ratios)
                
                print(f"  Results: Encoder={avg_encoder_time:.6f}s, Decoder={avg_decoder_time:.6f}s, CR={avg_compression_ratio:.6f} (from {len(encoder_times)} runs)")
                
                results.append({
                    'file': f"{test_file}.txt",
                    'processes': processes,
                    'avg_encoder_time': avg_encoder_time,
                    'avg_decoder_time': avg_decoder_time,
                    'avg_compression_ratio': avg_compression_ratio,
                    'runs': len(encoder_times)
                })
            else:
                print(f"All runs failed for {test_file}.txt with {processes} processes")
    
    # Save results to CSV
    if results:
        # Tạo thư mục Tests/Results nếu chưa có
        results_dir = os.path.join("..", "Tests", "Results")
        os.makedirs(results_dir, exist_ok=True)
        
        csv_file = os.path.join(results_dir, "mpi_benchmark_results.csv")
        
        print(f"\nSaving Results to {csv_file}")
        
        with open(csv_file, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            
            # Header
            writer.writerow(['File', 'Processes', 'Encoder_Time_seconds', 'Decoder_Time_seconds', 'Compression_Ratio', 'Runs'])
            
            # Data
            for result in results:
                writer.writerow([
                    result['file'],
                    result['processes'],
                    f"{result['avg_encoder_time']:.6f}",
                    f"{result['avg_decoder_time']:.6f}",
                    f"{result['avg_compression_ratio']:.6f}",
                    result['runs']
                ])
        
        print(f"Results saved to {csv_file}")
        
        # Display summary
        print(f"\nSummary")
        print(f"{'File':<15} {'Proc':<5} {'Encoder(s)':<12} {'Decoder(s)':<12} {'Comp.Ratio':<12} {'Status':<10}")
        print("-" * 80)
        
        for result in results:
            print(f"{result['file']:<15} {result['processes']:<5} {result['avg_encoder_time']:<12.6f} {result['avg_decoder_time']:<12.6f} {result['avg_compression_ratio']:<12.6f} {'Success':<10}")
            
    else:
        print("\nNo successful results to save")
    
    # Change back to original directory
    os.chdir(project_dir)
    print(f"\nBenchmark completed! Results saved in: {csv_file}")

if __name__ == "__main__":
    main()