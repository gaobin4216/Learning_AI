"""
三维水墨风格《清明上河图》- 可交互漫游版
多层视差 + 大气透视 + 相机漫游
"""

import pygame
import math
import random
import sys

pygame.init()

# 窗口设置
WIDTH, HEIGHT = 1400, 800
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("清明上河图 - 三维水墨漫游版")

# 画卷总宽度（用于滚动）
SCROLL_WIDTH = 4000

# ========== 水墨色彩体系 ==========
INK_BLACK = (30, 28, 25)
INK_DARK = (50, 48, 42)
INK_MEDIUM = (80, 75, 65)
INK_LIGHT = (120, 115, 100)
INK_WASH = (160, 155, 140)
INK_PALE = (200, 195, 180)
INK_MIST = (220, 215, 205)

VERMILLION = (190, 50, 40)
OCHRE = (180, 140, 80)
INDIGO = (60, 80, 120)
GREEN_WASH = (100, 130, 90)

PAPER_WHITE = (245, 240, 230)
PAPER_CREAM = (235, 228, 215)

# 字体
try:
    font_title = pygame.font.SysFont("simhei", 32)
    font_subtitle = pygame.font.SysFont("simhei", 16)
    font_small = pygame.font.SysFont("simhei", 14)
    font_hint = pygame.font.SysFont("simhei", 12)
except:
    font_title = pygame.font.Font(None, 32)
    font_subtitle = pygame.font.Font(None, 16)
    font_small = pygame.font.Font(None, 14)
    font_hint = pygame.font.Font(None, 12)


class Camera3D:
    """三维相机控制器"""

    def __init__(self):
        self.x = 0  # 画卷位置
        self.y = 0
        self.target_x = 0
        self.target_y = 0
        self.zoom = 1.0
        self.target_zoom = 1.0
        self.rotation = 0  # 轻微旋转增加立体感
        self.target_rotation = 0
        self.dragging = False
        self.drag_start = None
        self.auto_scroll = True
        self.auto_scroll_speed = 0.3

    def update(self):
        # 平滑插值
        self.x += (self.target_x - self.x) * 0.08
        self.y += (self.target_y - self.y) * 0.08
        self.zoom += (self.target_zoom - self.zoom) * 0.1
        self.rotation += (self.target_rotation - self.rotation) * 0.1

        # 自动滚动
        if self.auto_scroll and not self.dragging:
            self.target_x += self.auto_scroll_speed
            if self.target_x > SCROLL_WIDTH - WIDTH / self.zoom:
                self.target_x = 0

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:  # 左键拖动
                self.dragging = True
                self.drag_start = event.pos
                self.auto_scroll = False
            elif event.button == 4:  # 滚轮上 - 放大
                self.target_zoom = min(2.0, self.target_zoom + 0.1)
            elif event.button == 5:  # 滚轮下 - 缩小
                self.target_zoom = max(0.5, self.target_zoom - 0.1)

        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:
                self.dragging = False

        elif event.type == pygame.MOUSEMOTION:
            if self.dragging and self.drag_start:
                dx = event.pos[0] - self.drag_start[0]
                dy = event.pos[1] - self.drag_start[1]
                self.target_x -= dx / self.zoom
                self.target_y -= dy / self.zoom
                self.drag_start = event.pos

        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_LEFT:
                self.target_x -= 100
                self.auto_scroll = False
            elif event.key == pygame.K_RIGHT:
                self.target_x += 100
                self.auto_scroll = False
            elif event.key == pygame.K_UP:
                self.target_y -= 50
            elif event.key == pygame.K_DOWN:
                self.target_y += 50
            elif event.key == pygame.K_PLUS or event.key == pygame.K_EQUALS:
                self.target_zoom = min(2.0, self.target_zoom + 0.2)
            elif event.key == pygame.K_MINUS:
                self.target_zoom = max(0.5, self.target_zoom - 0.2)
            elif event.key == pygame.K_SPACE:
                self.auto_scroll = not self.auto_scroll
            elif event.key == pygame.K_r:  # 重置视角
                self.target_x = 0
                self.target_y = 0
                self.target_zoom = 1.0
                self.target_rotation = 0

    def apply(self, x, y, layer_depth=0):
        """应用相机变换，layer_depth: 0=最近, 1=最远"""
        # 视差效果：远的层移动慢
        parallax = 0.3 + 0.7 * (1 - layer_depth)

        # 变换坐标
        tx = (x - self.x * parallax) * self.zoom + WIDTH / 2
        ty = (y - self.y) * self.zoom + HEIGHT / 2

        return int(tx), int(ty)

    def get_visible_range(self):
        """获取可见范围"""
        left = self.x - WIDTH / (2 * self.zoom)
        right = self.x + WIDTH / (2 * self.zoom) + WIDTH / self.zoom
        return left, right


class ParallaxLayer:
    """视差图层"""

    def __init__(self, depth, elements):
        self.depth = depth  # 0=最近, 1=最远
        self.elements = elements
        self.fog_intensity = depth * 0.4  # 远处更雾

    def draw(self, surface, camera, time):
        left, right = camera.get_visible_range()

        # 大气透视：远处更淡
        fog_alpha = int(self.fog_intensity * 255)

        for elem in self.elements:
            # 视锥剔除
            if hasattr(elem, 'x'):
                if elem.x < left - 200 or elem.x > right + 200:
                    continue

            # 获取屏幕坐标
            if hasattr(elem, 'x') and hasattr(elem, 'y'):
                sx, sy = camera.apply(elem.x, elem.y, self.depth)
            else:
                sx, sy = 0, 0

            # 绘制元素
            if hasattr(elem, 'draw_at'):
                elem.draw_at(surface, sx, sy, camera.zoom, time, fog_alpha)


class InkBrush3D:
    """三维水墨笔触"""

    @staticmethod
    def stroke(surface, points, color=INK_BLACK, width=2, alpha=200):
        if len(points) < 2:
            return
        for i in range(len(points) - 1):
            t = i / len(points)
            w = width * (0.5 + 0.5 * math.sin(t * math.pi))
            alpha_surface = pygame.Surface((abs(points[i+1][0]-points[i][0])+int(w*2),
                                           abs(points[i+1][1]-points[i][1])+int(w*2)), pygame.SRCALPHA)
            start = (0, 0)
            end = (points[i+1][0]-points[i][0]+int(w), points[i+1][1]-points[i][1]+int(w))
            pygame.draw.line(alpha_surface, (*color, alpha), start, end, max(1, int(w)))
            surface.blit(alpha_surface, (min(points[i][0], points[i+1][0])-int(w),
                                         min(points[i][1], points[i+1][1])-int(w)))

    @staticmethod
    def dot(surface, x, y, radius, color=INK_BLACK, alpha=180):
        alpha_surface = pygame.Surface((radius*2, radius*2), pygame.SRCALPHA)
        pygame.draw.circle(alpha_surface, (*color, alpha), (radius, radius), radius)
        surface.blit(alpha_surface, (int(x-radius), int(y-radius)))


class WaterElement3D:
    """三维水面"""

    def __init__(self, x, y, width, depth):
        self.x = x
        self.y = y
        self.width = width
        self.depth = depth
        self.waves = []
        for _ in range(30):
            self.waves.append({
                'offset_x': random.randint(0, int(width)),
                'phase': random.uniform(0, math.pi*2),
                'speed': random.uniform(0.5, 1.5),
                'length': random.randint(15, 40)
            })

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        for wave in self.waves:
            wx = sx + int(wave['offset_x'] * zoom)
            wy = sy + math.sin(time * 2 + wave['phase']) * 3 * zoom
            length = int(wave['length'] * zoom)

            points = []
            for i in range(0, length, int(4*zoom)):
                px = wx + i
                py = wy + math.sin(time * 3 + i * 0.1 + wave['phase']) * 2 * zoom
                points.append((px, py))

            if len(points) >= 2:
                alpha = max(0, 80 - fog_alpha)
                InkBrush3D.stroke(surface, points, INK_WASH, 1, alpha)


class BuildingElement3D:
    """三维建筑"""

    def __init__(self, x, y, width, height, style="shop", depth=0.5):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.style = style
        self.depth = depth
        self.has_sign = random.random() > 0.5
        self.has_lantern = random.random() > 0.6
        self.lantern_phase = random.uniform(0, math.pi*2)
        self.window_glow = random.uniform(0, math.pi*2)

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        w = int(self.width * zoom)
        h = int(self.height * zoom)

        # 墙体
        wall_alpha = max(0, 180 - fog_alpha)
        alpha_surface = pygame.Surface((w, h), pygame.SRCALPHA)
        pygame.draw.rect(alpha_surface, (*INK_WASH, wall_alpha//2), (0, 0, w, h))
        surface.blit(alpha_surface, (sx, sy-h))

        # 墙线
        InkBrush3D.stroke(surface, [(sx, sy-h), (sx+w, sy-h), (sx+w, sy), (sx, sy), (sx, sy-h)],
                         INK_MEDIUM, max(1, int(2*zoom)), wall_alpha)

        # 屋顶
        roof_h = int(h * 0.35)
        roof_points = [
            (sx - int(w*0.08), sy-h),
            (sx + w//2, sy-h-roof_h),
            (sx + w + int(w*0.08), sy-h),
        ]
        InkBrush3D.stroke(surface, roof_points, INK_DARK, max(1, int(3*zoom)), wall_alpha)

        # 飞檐
        for dx in [-int(w*0.08), w+int(w*0.08)]:
            tip = (sx + dx, sy-h)
            curve = (sx + dx + (-int(12*zoom) if dx < 0 else int(12*zoom)), sy-h-int(roof_h*0.3))
            InkBrush3D.stroke(surface, [tip, curve], INK_DARK, max(1, int(2*zoom)), wall_alpha)

        # 窗户
        if self.style == "house":
            for i in range(2):
                wx = sx + int(w*0.2) + i * int(w*0.4)
                wy = sy - h + int(h*0.2)
                ww, wh = int(w*0.18), int(h*0.25)

                # 窗户发光效果
                glow = int(20 + 10 * math.sin(time * 2 + self.window_glow + i))
                window_alpha = max(0, 200 - fog_alpha)
                window_surface = pygame.Surface((ww, wh), pygame.SRCALPHA)
                pygame.draw.rect(window_surface, (255, 240, 180, window_alpha//2), (0, 0, ww, wh))
                surface.blit(window_surface, (wx, wy))

                pygame.draw.rect(surface, INK_LIGHT, (wx, wy, ww, wh), max(1, int(zoom)))

        # 灯笼
        if self.has_lantern:
            swing = math.sin(time * 1.2 + self.lantern_phase) * 4 * zoom
            lx = sx + int(w*0.12) + int(swing)
            ly = sy - h - int(roof_h*0.4)

            lantern_alpha = max(0, 220 - fog_alpha)
            pygame.draw.ellipse(surface, VERMILLION, (lx-int(6*zoom), ly, int(12*zoom), int(18*zoom)))


class BridgeElement3D:
    """三维虹桥"""

    def __init__(self, x, y, width, depth=0.4):
        self.x = x
        self.y = y
        self.width = width
        self.depth = depth
        self.arch_height = 90

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        w = int(self.width * zoom)
        arch_h = int(self.arch_height * zoom)

        bridge_alpha = max(0, 200 - fog_alpha)

        # 桥拱
        arch_points = []
        for i in range(0, w, max(1, int(3*zoom))):
            t = i / w
            arch_y = sy - int(arch_h * math.sin(t * math.pi))
            arch_points.append((sx + i, arch_y))

        if len(arch_points) >= 2:
            InkBrush3D.stroke(surface, arch_points, INK_DARK, max(1, int(4*zoom)), bridge_alpha)

        # 桥面
        surface_points = []
        for i in range(0, w, max(1, int(3*zoom))):
            t = i / w
            surf_y = sy - int(arch_h * math.sin(t * math.pi))
            surface_points.append((sx + i, surf_y))
        InkBrush3D.stroke(surface, surface_points, INK_MEDIUM, max(1, int(5*zoom)), bridge_alpha)

        # 桥栏杆
        for i in range(0, w, max(1, int(25*zoom))):
            t = i / w
            rail_y = sy - int(arch_h * math.sin(t * math.pi))

            # 立柱
            InkBrush3D.stroke(surface, [(sx+i, rail_y-int(25*zoom)), (sx+i, rail_y)],
                            INK_MEDIUM, max(1, int(2*zoom)), bridge_alpha)

            # 横杆
            if i > 0:
                prev_t = (i-int(25*zoom)) / w
                prev_y = sy - int(arch_h * math.sin(prev_t * math.pi))
                InkBrush3D.stroke(surface, [(sx+i-int(25*zoom), prev_y-int(18*zoom)), (sx+i, rail_y-int(18*zoom))],
                                INK_MEDIUM, max(1, int(zoom)), bridge_alpha)

        # 桥上的亭子
        pavilion_x = sx + w//2
        pavilion_y = sy - arch_h

        # 柱子
        for dx in [-int(30*zoom), int(30*zoom)]:
            InkBrush3D.stroke(surface,
                            [(pavilion_x+dx, pavilion_y),
                             (pavilion_x+dx, pavilion_y-int(45*zoom))],
                            INK_DARK, max(1, int(3*zoom)), bridge_alpha)

        # 亭顶
        roof_points = [
            (pavilion_x - int(50*zoom), pavilion_y - int(35*zoom)),
            (pavilion_x, pavilion_y - int(55*zoom)),
            (pavilion_x + int(50*zoom), pavilion_y - int(35*zoom)),
        ]
        InkBrush3D.stroke(surface, roof_points, INK_DARK, max(1, int(3*zoom)), bridge_alpha)

        # 飞檐
        for dx in [-int(50*zoom), int(50*zoom)]:
            tip = (pavilion_x + dx, pavilion_y - int(35*zoom))
            curve = (pavilion_x + dx + (-int(12*zoom) if dx < 0 else int(12*zoom)),
                    pavilion_y - int(47*zoom))
            InkBrush3D.stroke(surface, [tip, curve], INK_DARK, max(1, int(2*zoom)), bridge_alpha)

        # 桥上行人
        for i in range(6):
            px = sx + int(w * (0.15 + 0.7 * i/6))
            t = (px - sx) / w
            py = sy - int(arch_h * math.sin(t * math.pi))
            self._draw_person(surface, px, py-int(28*zoom), zoom, time+i, bridge_alpha)

    def _draw_person(self, surface, x, y, zoom, phase, alpha):
        sway = math.sin(phase * 0.5) * zoom
        # 头
        InkBrush3D.dot(surface, x+int(sway), y-int(8*zoom), max(1, int(3*zoom)), INK_DARK, alpha)
        # 身
        InkBrush3D.stroke(surface, [(x+int(sway), y-int(5*zoom)), (x+int(sway), y+int(5*zoom))],
                         INK_DARK, max(1, int(zoom)), alpha)
        # 腿
        leg = math.sin(phase) * 2 * zoom
        InkBrush3D.stroke(surface, [(x+int(sway), y+int(5*zoom)), (x-2*zoom+int(leg), y+int(12*zoom))],
                         INK_DARK, max(1, int(zoom)), alpha*0.8)
        InkBrush3D.stroke(surface, [(x+int(sway), y+int(5*zoom)), (x+2*zoom-int(leg), y+int(12*zoom))],
                         INK_DARK, max(1, int(zoom)), alpha*0.8)


class TreeElement3D:
    """三维树木"""

    def __init__(self, x, y, tree_type="willow", depth=0.5):
        self.x = x
        self.y = y
        self.type = tree_type
        self.depth = depth
        self.sway_phase = random.uniform(0, math.pi*2)
        self.size = random.uniform(0.8, 1.3)
        self.branches = self._generate_branches()

    def _generate_branches(self):
        branches = []
        num = random.randint(4, 8)
        for i in range(num):
            angle = -math.pi/2 + (i/num - 0.5) * math.pi * 0.8
            length = random.randint(25, 45) * self.size
            branches.append({'angle': angle, 'length': length})
        return branches

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        s = self.size * zoom
        tree_alpha = max(0, 200 - fog_alpha)
        sway = math.sin(time * 0.6 + self.sway_phase) * 3 * zoom

        if self.type == "willow":
            self._draw_willow(surface, sx, sy, s, zoom, sway, time, tree_alpha)
        else:
            self._draw_deciduous(surface, sx, sy, s, zoom, sway, time, tree_alpha)

    def _draw_willow(self, surface, tx, ty, s, zoom, sway, time, alpha):
        # 主干
        InkBrush3D.stroke(surface, [(tx, ty), (tx+int(sway*0.5), ty-int(70*s))],
                         INK_DARK, max(1, int(4*s)), alpha)

        # 柳枝
        crown_y = ty - int(65*s)
        for i in range(10):
            angle = (i/10) * math.pi * 1.2 - math.pi*0.1
            branch_len = int(50*s)
            end_x = tx + int(math.cos(angle) * branch_len) + int(sway)
            end_y = crown_y + int(math.sin(angle) * branch_len * 0.2)

            for j in range(6):
                t = j / 6
                droop = int(30*s * t * t)
                lx = int(tx + (end_x - tx) * t) + int(sway * t)
                ly = int(crown_y + (end_y - crown_y) * t) + droop
                if j > 0:
                    InkBrush3D.stroke(surface, [(prev_x, prev_y), (lx, ly)],
                                     GREEN_WASH, max(1, int(zoom)), int(alpha*0.7))
                prev_x, prev_y = lx, ly

    def _draw_deciduous(self, surface, tx, ty, s, zoom, sway, time, alpha):
        # 主干
        InkBrush3D.stroke(surface, [(tx, ty), (tx+int(sway*0.4), ty-int(55*s))],
                         INK_DARK, max(1, int(4*s)), alpha)

        # 树冠
        crown_x = tx + int(sway*0.4)
        crown_y = ty - int(50*s)

        for _ in range(12):
            leaf_x = crown_x + random.randint(-int(30*s), int(30*s))
            leaf_y = crown_y + random.randint(-int(25*s), int(10*s))
            radius = max(1, int(random.randint(5, 12) * s))
            InkBrush3D.dot(surface, leaf_x, leaf_y, radius, INK_MEDIUM, int(alpha*0.6))


class PersonElement3D:
    """三维人物"""

    def __init__(self, x, y, role="pedestrian", depth=0.5):
        self.x = x
        self.y = y
        self.role = role
        self.depth = depth
        self.speed = random.uniform(0.3, 1.0)
        self.direction = random.choice([-1, 1])
        self.walk_phase = random.uniform(0, math.pi*2)
        self.size = random.uniform(0.7, 1.0)
        self.color = random.choice([INK_DARK, INK_MEDIUM, VERMILLION, INDIGO])

    def update(self, time):
        self.x += self.speed * self.direction
        self.walk_phase += 0.08
        if self.x > SCROLL_WIDTH + 30:
            self.x = -30
        elif self.x < -30:
            self.x = SCROLL_WIDTH + 30

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        s = self.size * zoom
        person_alpha = max(0, 200 - fog_alpha)
        leg_swing = math.sin(self.walk_phase) * 4 * zoom

        # 头
        InkBrush3D.dot(surface, sx, sy-int(22*s), max(1, int(4*s)), INK_DARK, person_alpha)
        # 身体
        InkBrush3D.stroke(surface, [(sx, sy-int(18*s)), (sx, sy-int(2*s))],
                         self.color, max(1, int(2*s)), person_alpha)
        # 腿
        InkBrush3D.stroke(surface, [(sx, sy-int(2*s)), (sx-2*zoom+int(leg_swing), sy+int(8*s))],
                         INK_DARK, max(1, int(1.5*s)), int(person_alpha*0.8))
        InkBrush3D.stroke(surface, [(sx, sy-int(2*s)), (sx+2*zoom-int(leg_swing), sy+int(8*s))],
                         INK_DARK, max(1, int(1.5*s)), int(person_alpha*0.8))


class BoatElement3D:
    """三维船只"""

    def __init__(self, x, y, size=1.0, direction=1, depth=0.4):
        self.x = x
        self.y = y
        self.size = size
        self.direction = direction
        self.depth = depth
        self.speed = random.uniform(0.2, 0.6) * direction
        self.wobble_phase = random.uniform(0, math.pi*2)
        self.has_cabin = random.random() > 0.3
        self.has_sail = random.random() > 0.5
        self.people = random.randint(1, 3)

    def update(self, time):
        self.x += self.speed
        if self.x > SCROLL_WIDTH + 100:
            self.x = -100
        elif self.x < -100:
            self.x = SCROLL_WIDTH + 100

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        s = self.size * zoom
        boat_alpha = max(0, 200 - fog_alpha)
        wobble = math.sin(time * 1.5 + self.wobble_phase) * 2 * zoom

        # 船身
        hull_points = [
            (sx - int(50*s), sy + int(8*s) + int(wobble)),
            (sx - int(45*s), sy + int(20*s) + int(wobble)),
            (sx + int(45*s), sy + int(20*s) + int(wobble)),
            (sx + int(50*s), sy + int(8*s) + int(wobble)),
        ]
        for i in range(len(hull_points)-1):
            InkBrush3D.stroke(surface, [hull_points[i], hull_points[i+1]],
                            INK_DARK, max(1, int(2*s)), boat_alpha)

        # 船舱
        if self.has_cabin:
            cabin_y = sy - int(5*s) + int(wobble)
            InkBrush3D.stroke(surface,
                            [(sx-int(20*s), cabin_y), (sx+int(20*s), cabin_y)],
                            INK_MEDIUM, max(1, int(2*s)), boat_alpha)

            # 船篷
            roof_points = [
                (sx - int(25*s), cabin_y),
                (sx, cabin_y - int(15*s)),
                (sx + int(25*s), cabin_y),
            ]
            InkBrush3D.stroke(surface, roof_points, INK_DARK, max(1, int(2*s)), boat_alpha)

        # 帆
        if self.has_sail:
            mast_x = sx + int(30*s * self.direction)
            mast_y = sy + int(wobble)
            InkBrush3D.stroke(surface, [(mast_x, mast_y+int(5*s)), (mast_x, mast_y-int(35*s))],
                            INK_DARK, max(1, int(2*s)), boat_alpha)

        # 船上的人
        for i in range(self.people):
            px = sx + int((-15 + i*12) * s)
            py = sy - int(5*s) + int(wobble)
            InkBrush3D.dot(surface, px, py-int(10*s), max(1, int(3*s)), INK_DARK, boat_alpha)
            InkBrush3D.stroke(surface, [(px, py-int(7*s)), (px, py)],
                            INK_DARK, max(1, int(s)), boat_alpha)


class GateElement3D:
    """三维城门"""

    def __init__(self, x, y, width, height, depth=0.4):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.depth = depth

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        w = int(self.width * zoom)
        h = int(self.height * zoom)
        gate_alpha = max(0, 200 - fog_alpha)

        # 城墙
        wall_w = int(200 * zoom)
        pygame.draw.rect(surface, INK_WASH, (sx-wall_w, sy-h, wall_w, h+int(100*zoom)))
        pygame.draw.rect(surface, INK_WASH, (sx+w, sy-h, wall_w, h+int(100*zoom)))

        # 城墙轮廓
        InkBrush3D.stroke(surface, [(sx-wall_w, sy-h), (sx-wall_w, sy+int(100*zoom)),
                                   (sx, sy+int(100*zoom))],
                         INK_MEDIUM, max(1, int(2*zoom)), gate_alpha)
        InkBrush3D.stroke(surface, [(sx+w, sy-h), (sx+w, sy+int(100*zoom)),
                                   (sx+w+wall_w, sy+int(100*zoom))],
                         INK_MEDIUM, max(1, int(2*zoom)), gate_alpha)

        # 城门洞
        arch_rect = (sx, sy-int(h*0.3), w, int(h*0.6))
        pygame.draw.arc(surface, INK_MEDIUM, arch_rect, 0, math.pi, max(1, int(2*zoom)))

        # 城楼
        tower_h = int(80 * zoom)
        tower_y = sy - h - tower_h

        # 城楼主体
        pygame.draw.rect(surface, INK_WASH, (sx, tower_y, w, tower_h))

        # 城楼屋顶
        roof_points = [
            (sx - int(20*zoom), tower_y),
            (sx + w//2, tower_y - int(50*zoom)),
            (sx + w + int(20*zoom), tower_y),
        ]
        InkBrush3D.stroke(surface, roof_points, INK_DARK, max(1, int(3*zoom)), gate_alpha)

        # 飞檐
        for dx in [-int(20*zoom), w+int(20*zoom)]:
            tip = (sx + dx, tower_y)
            curve = (sx + dx + (-int(15*zoom) if dx < 0 else int(15*zoom)), tower_y - int(15*zoom))
            InkBrush3D.stroke(surface, [tip, curve], INK_DARK, max(1, int(2*zoom)), gate_alpha)

        # 城门匾额
        plaque_w = int(60 * zoom)
        plaque_h = int(20 * zoom)
        plaque_x = sx + w//2 - plaque_w//2
        plaque_y = sy - int(h*0.3) - plaque_h - int(5*zoom)
        pygame.draw.rect(surface, VERMILLION, (plaque_x, plaque_y, plaque_w, plaque_h))


def create_scene():
    """创建完整三维场景"""
    layers = {
        'far_mountains': [],
        'mid_mountains': [],
        'far_buildings': [],
        'north_trees': [],
        'north_buildings': [],
        'north_people': [],
        'water': [],
        'bridge': [],
        'boats': [],
        'gate': [],
        'south_buildings': [],
        'south_trees': [],
        'south_people': [],
    }

    # 远山
    for i in range(2):
        for x in range(0, SCROLL_WIDTH, 100):
            layers['far_mountains'].append(
                MountainElement(x + random.randint(-20, 20),
                              HEIGHT*0.3 + random.randint(-10, 10),
                              random.randint(80, 150), 0.9))

    # 中景山
    for x in range(0, SCROLL_WIDTH, 150):
        layers['mid_mountains'].append(
            MountainElement(x + random.randint(-30, 30),
                          HEIGHT*0.35 + random.randint(-15, 15),
                          random.randint(60, 120), 0.7))

    # 北岸建筑
    x = 100
    while x < SCROLL_WIDTH - 200:
        w = random.randint(70, 130)
        h = random.randint(60, 100)
        y = HEIGHT*0.42
        style = random.choice(["shop", "house", "shop"])
        layers['north_buildings'].append(BuildingElement3D(x, y, w, h, style, 0.5))
        x += w + random.randint(5, 25)

    # 北岸树木
    for i in range(30):
        x = random.randint(50, SCROLL_WIDTH-50)
        y = HEIGHT*0.40 + random.randint(-5, 5)
        layers['north_trees'].append(TreeElement3D(x, y, "willow", 0.45))

    # 北岸行人
    for i in range(60):
        x = random.randint(0, SCROLL_WIDTH)
        y = HEIGHT*0.44 + random.randint(-8, 8)
        role = random.choice(["pedestrian", "merchant", "official"])
        layers['north_people'].append(PersonElement3D(x, y, role, 0.45))

    # 水面
    layers['water'].append(WaterElement3D(0, HEIGHT*0.55, SCROLL_WIDTH, 0.4))

    # 虹桥
    layers['bridge'].append(BridgeElement3D(SCROLL_WIDTH*0.35, HEIGHT*0.62, 300, 0.38))

    # 船只
    for i in range(12):
        x = random.randint(200, SCROLL_WIDTH-200)
        y = HEIGHT*0.56 + random.randint(-15, 15)
        direction = random.choice([-1, 1])
        layers['boats'].append(BoatElement3D(x, y, 0.9, direction, 0.38))

    # 城门
    layers['gate'].append(GateElement3D(SCROLL_WIDTH*0.75, HEIGHT*0.45, 100, 130, 0.35))

    # 南岸建筑
    x = 150
    while x < SCROLL_WIDTH - 200:
        w = random.randint(80, 140)
        h = random.randint(70, 110)
        y = HEIGHT*0.72
        style = random.choice(["shop", "house"])
        layers['south_buildings'].append(BuildingElement3D(x, y, w, h, style, 0.3))
        x += w + random.randint(10, 30)

    # 南岸树木
    for i in range(25):
        x = random.randint(50, SCROLL_WIDTH-50)
        y = HEIGHT*0.70 + random.randint(-5, 5)
        layers['south_trees'].append(TreeElement3D(x, y, "deciduous", 0.28))

    # 南岸行人
    for i in range(40):
        x = random.randint(0, SCROLL_WIDTH)
        y = HEIGHT*0.74 + random.randint(-8, 8)
        role = random.choice(["pedestrian", "merchant"])
        layers['south_people'].append(PersonElement3D(x, y, role, 0.28))

    return layers


class MountainElement:
    """山元素"""
    def __init__(self, x, y, height, depth):
        self.x = x
        self.y = y
        self.height = height
        self.depth = depth
        self.peaks = [(random.randint(-40, 40), random.randint(-height, -height//2)) for _ in range(3)]

    def draw_at(self, surface, sx, sy, zoom, time, fog_alpha):
        mountain_alpha = max(0, int(100 - fog_alpha * 0.8))

        # 山体轮廓
        points = [(sx-int(60*zoom), sy)]
        for px, py in self.peaks:
            points.append((sx+int(px*zoom), sy+int(py*zoom)))
        points.append((sx+int(60*zoom), sy))

        if len(points) >= 3:
            alpha_surface = pygame.Surface((int(150*zoom), int(self.height*zoom*1.5)), pygame.SRCALPHA)
            local_points = [(p[0]-sx+int(75*zoom), p[1]-sy+int(self.height*zoom)) for p in points]
            pygame.draw.polygon(alpha_surface, (*INK_PALE, mountain_alpha), local_points)
            surface.blit(alpha_surface, (sx-int(75*zoom), sy-int(self.height*zoom)))


def draw_paper_background(surface):
    """宣纸背景"""
    surface.fill(PAPER_WHITE)

    for _ in range(300):
        x = random.randint(0, WIDTH)
        y = random.randint(0, HEIGHT)
        size = random.randint(1, 3)
        alpha = random.randint(10, 25)
        color = random.choice([PAPER_CREAM, INK_PALE])
        alpha_surface = pygame.Surface((size, size), pygame.SRCALPHA)
        pygame.draw.rect(alpha_surface, (*color, alpha), (0, 0, size, size))
        surface.blit(alpha_surface, (x, y))


def draw_ui(surface, camera, time):
    """绘制UI界面"""
    # 标题
    title_bg = pygame.Surface((250, 70), pygame.SRCALPHA)
    pygame.draw.rect(title_bg, (*PAPER_CREAM, 200), (0, 0, 250, 70))
    pygame.draw.rect(title_bg, (*INK_LIGHT, 150), (0, 0, 250, 70), 2)
    surface.blit(title_bg, (10, 10))

    title = font_title.render("清明上河图", True, INK_BLACK)
    surface.blit(title, (25, 18))

    subtitle = font_subtitle.render("北宋·张择端  三维水墨漫游版", True, INK_MEDIUM)
    surface.blit(subtitle, (25, 50))

    # 印章
    seal_x, seal_y = 210, 25
    pygame.draw.rect(surface, VERMILLION, (seal_x, seal_y, 35, 35))
    seal_text = font_small.render("印", True, PAPER_WHITE)
    surface.blit(seal_text, (seal_x+10, seal_y+8))

    # 操作提示
    hints = [
        "← → / 拖动: 漫游画卷",
        "滚轮 / +/-: 缩放",
        "空格: 自动/手动切换",
        "R: 重置视角",
        "ESC: 退出",
    ]

    hint_bg = pygame.Surface((180, len(hints)*20+10), pygame.SRCALPHA)
    pygame.draw.rect(hint_bg, (*PAPER_CREAM, 180), (0, 0, 180, len(hints)*20+10))
    surface.blit(hint_bg, (WIDTH-190, 10))

    for i, hint in enumerate(hints):
        text = font_hint.render(hint, True, INK_MEDIUM)
        surface.blit(text, (WIDTH-185, 15 + i*20))

    # 状态信息
    status = f"位置: {int(camera.x)}  缩放: {camera.zoom:.1f}x"
    if camera.auto_scroll:
        status += "  [自动滚动]"
    status_text = font_small.render(status, True, INK_LIGHT)
    surface.blit(status_text, (10, HEIGHT-25))


def main():
    clock = pygame.time.Clock()
    time_val = 0

    camera = Camera3D()
    layers = create_scene()

    # 预绘制背景
    background = pygame.Surface((WIDTH, HEIGHT))
    draw_paper_background(background)

    # 图层顺序（从远到近）
    layer_order = [
        'far_mountains', 'mid_mountains',
        'far_buildings',
        'north_trees', 'north_buildings', 'north_people',
        'water', 'boats', 'bridge', 'gate',
        'south_buildings', 'south_trees', 'south_people'
    ]

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
            camera.handle_event(event)

        time_val += 0.016
        camera.update()

        # 更新可移动元素
        for person in layers.get('north_people', []):
            person.update(time_val)
        for person in layers.get('south_people', []):
            person.update(time_val)
        for boat in layers.get('boats', []):
            boat.update(time_val)

        # 绘制
        screen.blit(background, (0, 0))

        # 按层次绘制
        for layer_name in layer_order:
            if layer_name in layers:
                for elem in layers[layer_name]:
                    # 视锥剔除
                    left, right = camera.get_visible_range()
                    if hasattr(elem, 'x'):
                        if elem.x < left - 300 or elem.x > right + 300:
                            continue

                    # 计算屏幕坐标
                    if hasattr(elem, 'x') and hasattr(elem, 'y'):
                        sx, sy = camera.apply(elem.x, elem.y, elem.depth if hasattr(elem, 'depth') else 0.5)

                        # 大气透视
                        fog_alpha = int((elem.depth if hasattr(elem, 'depth') else 0.5) * 150)

                        # 缩放调整
                        zoom = camera.zoom

                        if hasattr(elem, 'draw_at'):
                            elem.draw_at(screen, sx, sy, zoom, time_val, fog_alpha)

        # UI
        draw_ui(screen, camera, time_val)

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()


if __name__ == "__main__":
    main()
