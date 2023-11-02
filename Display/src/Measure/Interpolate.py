import numpy as np

# Hiệu điện thế (V)
voltages = np.array([0, 3, 6, 9, 12, 15, 24, 110, 223])

# Biên độ sóng tương ứng
amplitudes = np.array([
    np.mean([5, 6, 8, 9, 7, 10]),
    np.mean([14, 15, 16, 17, 18]),
    np.mean([17, 18, 19, 20, 21]),
    np.mean([23, 24, 25, 26, 27]),
    np.mean([26, 27, 28, 29, 30]),
    np.mean([33, 34, 35, 36]),
    np.mean([42, 43, 44, 45]),
    np.mean([178, 179, 180, 181, 182, 183]),
    np.mean([355, 356, 370, 373, 377])
])

# # Chuyển đổi biên độ sóng thành hiệu điện thế
# amplitudes_in_volts = amplitudes * (5 / 1023) * 5

# Tính toán hệ số hồi quy tuyến tính
coefficients = np.polyfit(amplitudes , voltages , 1)

print(f"Công thức liên hệ giữa hiệu điện thế và biên độ sóng: V = {coefficients[0]}*A + {coefficients[1]}")

#Công thức liên hệ giữa hiệu điện thế và biên độ sóng: V = 0.6271758831740704*A + -5.521341229107278