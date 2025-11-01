import pygame
import math
import time

# choose bot
bot = "turtlebot/"
# bot = "minibot/"

# choose file
filename = "box"
# filename = "ball_static"
# filename = "ball_moving"

input_file = bot + "segmented_data/" + filename + "_segmented.dat"

output_file = bot + "labeled_data/" + filename + "_labeled.dat"

WINDOW_SIZE = (800, 800)
LABELED_COLOR = (255, 0, 0)
MOUSE_RADIUS = 50
INITIAL_SCALE = 60
CENTER = [WINDOW_SIZE[0] // 2, WINDOW_SIZE[1] // 2]

AUTO_SCROLL_DELAY = 0.5
AUTO_SCROLL_INTERVAL = 0.1

SEGMENT_COLORS = [
    (0, 255, 0), (0, 0, 255), (255, 255, 0),
    (0, 255, 255), (255, 0, 255), (255, 165, 0),
    (128, 0, 128), (0, 128, 0), (0, 0, 128),
]

def load_segmented_data(filename):
    all_seconds = {}
    with open(filename, "r") as f:
        sec = 0
        while True:
            line = f.readline()
            if not line:
                break
            line = line.strip()
            if not line:
                continue

            n = int(line)
            segments = []
            for _ in range(n):
                vals = list(map(float, f.readline().strip().split()))
                a_i = int(vals[0])
                coords = [(vals[2*i+1], vals[2*i+2]) for i in range(a_i)]
                segments.append(coords)
            all_seconds[sec] = segments
            sec += 1
    print(f"Loaded {len(all_seconds)} seconds from {filename}")
    return all_seconds

def save_labeled_segments(labeled_data, output_file):
    with open(output_file, "w") as f:
        for t in sorted(labeled_data.keys()):
            segments, labels = labeled_data[t]
            f.write(f"{len(segments)}\n")
            for seg, label in zip(segments, labels):
                f.write(f"{len(seg)} {label}")
                for (x, y) in seg:
                    f.write(f" {x} {y}")
                f.write("\n")
            f.write("\n")
    print(f"Labeled data saved to {output_file}")

def transform_points(segments, scale, offset_x, offset_y):
    screen_segments = []
    for seg in segments:
        screen_seg = []
        for (x, y) in seg:
            sx = int(CENTER[0] + x * scale + offset_x)
            sy = int(CENTER[1] - y * scale + offset_y)
            screen_seg.append((sx, sy))
        screen_segments.append(screen_seg)
    return screen_segments

def main():
    pygame.init()
    screen = pygame.display.set_mode(WINDOW_SIZE)
    font = pygame.font.SysFont("Arial", 24)
    clock = pygame.time.Clock()

    data = load_segmented_data(input_file)
    seconds = sorted(data.keys())
    if not seconds:
        print("No data found.")
        return

    labeled_data = {}
    current_index = 0
    scale = INITIAL_SCALE
    offset_x, offset_y = 0, 0

    def load_sec(sec):
        segments = data[sec]
        labels = [0] * len(segments)
        if sec in labeled_data:
            labels = labeled_data[sec][1]
        screen_segments = transform_points(segments, scale, offset_x, offset_y)
        labeled_data[sec] = (segments, labels)
        return segments, labels, screen_segments

    segments, labels, screen_segments = load_sec(seconds[current_index])
    dragging = False
    last_mouse = None
    key_hold_start = None
    last_auto_scroll = 0
    held_key = None
    running = True

    while running:
        sec = seconds[current_index]
        screen.fill((0, 0, 0))
        time_text = font.render(f"Time: {sec}s  Scale: {scale:.3f}", True, (255, 255, 255))
        screen.blit(time_text, (20, 20))

        keys = pygame.key.get_pressed()
        ctrl_held = keys[pygame.K_LCTRL] or keys[pygame.K_RCTRL]
        mouse_x, mouse_y = pygame.mouse.get_pos()

        if not ctrl_held:
            circle_surface = pygame.Surface(WINDOW_SIZE, pygame.SRCALPHA)
            pygame.draw.circle(circle_surface, (255, 255, 255, 64), (mouse_x, mouse_y), MOUSE_RADIUS)
            screen.blit(circle_surface, (0, 0))

        for seg_idx, seg in enumerate(screen_segments):
            color = LABELED_COLOR if labels[seg_idx] == 1 else SEGMENT_COLORS[seg_idx % len(SEGMENT_COLORS)]
            for pt in seg:
                pygame.draw.circle(screen, color, pt, 2)

        current_time = time.time()
        if keys[pygame.K_RIGHT] or keys[pygame.K_LEFT]:
            if held_key is None:
                held_key = pygame.K_RIGHT if keys[pygame.K_RIGHT] else pygame.K_LEFT
                key_hold_start = current_time
            else:
                if current_time - key_hold_start > AUTO_SCROLL_DELAY:
                    if current_time - last_auto_scroll > AUTO_SCROLL_INTERVAL:
                        if held_key == pygame.K_RIGHT and current_index < len(seconds) - 1:
                            current_index += 1
                        elif held_key == pygame.K_LEFT and current_index > 0:
                            current_index -= 1
                        segments, labels, screen_segments = load_sec(seconds[current_index])
                        last_auto_scroll = current_time
        else:
            held_key = None
            key_hold_start = None

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                save_labeled_segments(labeled_data, output_file)
                pygame.quit()
                return

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    if current_index < len(seconds) - 1:
                        current_index += 1
                        segments, labels, screen_segments = load_sec(seconds[current_index])
                    else:
                        save_labeled_segments(labeled_data, output_file)
                        pygame.quit()
                        return
                elif event.key == pygame.K_RIGHT:
                    if current_index < len(seconds) - 1:
                        current_index += 1
                        segments, labels, screen_segments = load_sec(seconds[current_index])
                elif event.key == pygame.K_LEFT:
                    if current_index > 0:
                        current_index -= 1
                        segments, labels, screen_segments = load_sec(seconds[current_index])
                elif event.key in (pygame.K_EQUALS, pygame.K_PLUS, pygame.K_UP):
                    scale *= 1.1
                    screen_segments = transform_points(segments, scale, offset_x, offset_y)
                elif event.key in (pygame.K_MINUS, pygame.K_DOWN):
                    scale /= 1.1
                    screen_segments = transform_points(segments, scale, offset_x, offset_y)

            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 4:
                    scale *= 1.1
                    screen_segments = transform_points(segments, scale, offset_x, offset_y)
                elif event.button == 5:
                    scale /= 1.1
                    screen_segments = transform_points(segments, scale, offset_x, offset_y)
                elif event.button == 1:
                    if ctrl_held:
                        dragging = True
                        last_mouse = (mouse_x, mouse_y)
                    else:
                        for i, seg in enumerate(screen_segments):
                            if any(math.hypot(px - mouse_x, py - mouse_y) <= MOUSE_RADIUS for (px, py) in seg):
                                labels[i] = 1
                elif event.button == 3:
                    for i, seg in enumerate(screen_segments):
                        if any(math.hypot(px - mouse_x, py - mouse_y) <= MOUSE_RADIUS for (px, py) in seg):
                            labels[i] = 0

            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1 and dragging:
                    dragging = False
                    last_mouse = None

            elif event.type == pygame.MOUSEMOTION:
                if dragging and ctrl_held and last_mouse is not None:
                    dx = mouse_x - last_mouse[0]
                    dy = mouse_y - last_mouse[1]
                    offset_x += dx
                    offset_y += dy
                    screen_segments = transform_points(segments, scale, offset_x, offset_y)
                    last_mouse = (mouse_x, mouse_y)

        pygame.display.flip()
        clock.tick(60)


if __name__ == "__main__":
    main()
