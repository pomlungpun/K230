# ==============================
# K230 CanMV - Red Stop / Red Lost -> Go (15)
# ==============================
import time, os, gc, sys
from media.sensor import *
from media.display import *
from media.media import *
from ybUtils.YbUart import YbUart

print(">>> System Starting...")

# ---------- UART ----------
try:
    uart = YbUart(baudrate=9600)
    uart.send("Hello Yahboom\n")
    print("UART Initialized")
except Exception as e:
    print("UART Error:", e)

# ---------- Camera Config ----------
DETECT_WIDTH  = 320
DETECT_HEIGHT = 240

# Threshold สีแดง (LAB)
red_threshold   = (5, 95, 15, 127, 15, 127)

# ตัวแปรสำหรับ DataMatrix
last_dm_payload = ""

# ตัวแปรกันกระพริบ (Debounce)
detect_counter = 0
DETECT_THRESHOLD = 5      # เจอต่อเนื่อง 5 เฟรมถึงนับ
current_stable_color = 0  # 0=ไม่เจอ, 16=แดง
last_sent_color = -1      # จำค่าล่าสุดที่ส่งไป UART

sensor = None

try:
    print("Initializing Sensor...")
    sensor = Sensor(width=DETECT_WIDTH, height=DETECT_HEIGHT)
    sensor.reset()
    sensor.set_framesize(width=DETECT_WIDTH, height=DETECT_HEIGHT)
    sensor.set_pixformat(Sensor.RGB565)

    try:
        sensor.set_auto_gain(True)
        sensor.set_auto_whitebal(True)
    except: pass

    Display.init(Display.VIRT, width=DETECT_WIDTH, height=DETECT_HEIGHT, fps=30)
    MediaManager.init()
    sensor.run()

    print(">>> System Ready - Red Stop / Lost -> Go(15)")

    while True:
        os.exitpoint()
        img = sensor.snapshot()

        # ----------------------------------------
        # 1. DataMatrix
        # ----------------------------------------
        matrices = img.find_datamatrices()
        if matrices:
            for matrix in matrices:
                payload = matrix.payload()
                img.draw_rectangle(matrix.rect(), color=(0, 0, 0), thickness=3)
                img.draw_string_advanced(matrix.x(), matrix.y()-20, 20, payload, color=(0,0,255))

                if payload != last_dm_payload:
                    uart.send(payload + "\n")
                    print("DM SEND:", payload)
                    last_dm_payload = payload

        # ----------------------------------------
        # 2. Red Detection (หาเฉพาะสีแดง)
        # ----------------------------------------
        frame_color_found = 0

        def is_square(blob):
            if blob.w() < 10 or blob.h() < 10: return False
            ratio = blob.w() / blob.h()
            if not (0.60 <= ratio <= 1.50): return False
            if (blob.pixels() / (blob.w() * blob.h())) < 0.45: return False
            return True

        blobs_r = img.find_blobs([red_threshold], pixels_threshold=200, area_threshold=200, merge=True)
        if blobs_r:
            largest_r = max(blobs_r, key=lambda b: b.pixels())
            if is_square(largest_r):
                img.draw_rectangle(largest_r.rect(), color=(255, 0, 0), thickness=2)
                img.draw_cross(largest_r.cx(), largest_r.cy(), color=(255, 0, 0))
                frame_color_found = 16  # เฟรมนี้เจอแดง

        # ----------------------------------------
        # 3. Debounce & State Machine
        # ----------------------------------------

        # เพิ่ม/ลด ตัวนับความมั่นใจ
        if frame_color_found == 16:
            if detect_counter < DETECT_THRESHOLD + 2:
                detect_counter += 1
        else:
            if detect_counter > 0:
                detect_counter -= 1

        # ตัดสินใจสถานะ (Stable State)
        if detect_counter >= DETECT_THRESHOLD:
            current_stable_color = 16   # มั่นใจว่าเจอแดง
        elif detect_counter == 0:
            current_stable_color = 0    # มั่นใจว่าไม่เจออะไรเลย

        # ----------------------------------------
        # 4. UART Sending Logic (แก้ไขตรงนี้)
        # ----------------------------------------

        # กรณี 1: เจอแดง (Stop)
        if current_stable_color == 16:
            # ส่งแค่ครั้งแรกที่เจอ
            if last_sent_color != 16:
                uart.send("16\n")
                print(">>> RED DETECTED: Sent 16 (STOP)")
                last_sent_color = 16

        # กรณี 2: ไม่เจอแดง (Go) -- *สำคัญ: เช็คว่าเพิ่งหลุดจากแดงหรือไม่*
        elif current_stable_color == 0:
            # ถ้าสถานะก่อนหน้าคือ 16 (แดง) แสดงว่า "แดงเพิ่งหายไป"
            if last_sent_color == 16:
                uart.send("15\n")   # ส่งรหัสปลดล็อค (แทนสีเขียว)
                print(">>> RED LOST: Sent 15 (GO)")
                last_sent_color = 15  # เปลี่ยนสถานะเป็น 15 เพื่อไม่ให้ส่งซ้ำรัวๆ

        Display.show_image(img)
        gc.collect()

except BaseException as e:
    print("CRITICAL ERROR:", e)
    import time
    time.sleep(5)
finally:
    if isinstance(sensor, Sensor): sensor.stop()
    Display.deinit()
    MediaManager.deinit()
    try: uart.send("STOP\n")
    except: pass
    os.exitpoint(os.exitpoint(os.EXITPOINT_ENABLE_SLEEP))
