def filter_ascii_only(input_file, output_file):
    """Lọc file chỉ giữ các byte 0-127 (ASCII)"""
    try:
        with open(input_file, 'rb') as f:
            data = f.read()
        
        # Lọc chỉ giữ bytes < 128
        ascii_data = bytes(b for b in data if b < 128)
        
        with open(output_file, 'wb') as f:
            f.write(ascii_data)
        
        print(f"Input file: {input_file}")
        print(f"Output file: {output_file}")
        print(f"Original bytes: {len(data):,}")
        print(f"ASCII-only bytes: {len(ascii_data):,}")
        print(f"Removed bytes: {len(data) - len(ascii_data):,}")
        print(f"Percentage kept: {len(ascii_data)/len(data)*100:.2f}%")
        
        return True
        
    except Exception as e:
        print(f"Error: {e}")
        return False

if __name__ == "__main__":
    # Lọc shakespeare.txt
    input_path = "Tests/shakespeare.txt"
    output_path = "Tests/shakespeare_ascii.txt"
    
    print("Filtering")
    success = filter_ascii_only(input_path, output_path)
    
    if success:
        print("\nFiltering completed successfully!")
    else:
        print("Filtering failed!")