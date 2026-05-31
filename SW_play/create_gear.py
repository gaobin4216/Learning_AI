"""
SolidWorks 2022 齿轮零件自动绘制脚本
通过 COM 接口控制 SolidWorks，创建标准渐开线直齿圆柱齿轮（简化梯形齿形）
"""

import win32com.client
import math
import time


def main():
    # ==================== 齿轮参数（可修改）====================
    z = 20           # 齿数
    m = 3            # 模数 (mm)
    alpha = 20       # 压力角 (度)
    face_width = 20  # 面宽/齿宽 (mm)
    bore_dia = 20    # 中心孔径 (mm)

    # ==================== 计算齿轮尺寸 ====================
    pitch_r = m * z / 2              # 分度圆半径 30mm
    addendum = m                     # 齿顶高 = 模数
    dedendum = 1.25 * m              # 齿根高 = 1.25 * 模数
    outer_r = pitch_r + addendum     # 齿顶圆半径 33mm
    root_r = pitch_r - dedendum      # 齿根圆半径 26.25mm
    tooth_angle = 2 * math.pi / z    # 每齿角度
    alpha_rad = math.radians(alpha)  # 压力角弧度

    print(f"齿轮参数: 齿数={z}, 模数={m}mm, 压力角={alpha}°")
    print(f"分度圆半径={pitch_r}mm, 齿顶圆半径={outer_r}mm, 齿根圆半径={root_r}mm")
    print(f"面宽={face_width}mm, 中心孔径={bore_dia}mm")

    # ==================== 连接 SolidWorks ====================
    print("正在连接 SolidWorks...")
    try:
        swApp = win32com.client.Dispatch("SldWorks.Application")
    except Exception as e:
        print(f"无法连接 SolidWorks: {e}")
        print("请确保 SolidWorks 2022 已启动")
        return

    swApp.Visible = True
    time.sleep(1)

    # ==================== 新建零件 ====================
    print("正在创建新零件...")
    template = swApp.GetUserPreferenceStringValue(8)  # swDefaultTemplatePart
    if not template:
        # 常见的 SolidWorks 2022 默认模板路径
        template = r"C:\ProgramData\SolidWorks\SOLIDWORKS 2022\templates\Part.prtdot"

    model = swApp.NewDocument(template, 0, 0, 0)
    if model is None:
        print("无法创建新零件文档，请检查模板路径")
        return

    print("零件文档创建成功")

    # ==================== 第一步：创建齿轮基体圆盘 ====================
    print("正在绘制基体圆盘轮廓...")
    sketchMgr = model.SketchManager

    # 获取 Front Plane
    feat = model.FirstFeature()
    front_plane = None
    while feat:
        name = feat.Name
        if "Front" in name or "前视" in name:
            front_plane = feat
            break
        feat = feat.GetNextFeature()

    if front_plane is None:
        print("未找到 Front Plane，尝试使用默认基准面")
        # 尝试通过 SelectByID2 选择
        selMgr = model.SelectionManager
        model.Extension.SelectByID2("Front Plane", "PLANE", 0, 0, 0, False, 0, None, 0)
        front_plane = selMgr.GetSelectedObject6(1, -1)

    if front_plane is None:
        print("错误：无法获取基准面")
        return

    # 在 Front Plane 上创建草图
    front_plane.Select2(False, 0)
    sketchMgr.InsertSketch(True)

    # 绘制齿根圆（基体外径）
    sketchMgr.CreateCircleByRadius(0, 0, 0, root_r)
    # 绘制中心孔
    sketchMgr.CreateCircleByRadius(0, 0, 0, bore_dia / 2)

    sketchMgr.InsertSketch(True)
    print("基体圆盘轮廓绘制完成")

    # 拉伸基体
    print("正在拉伸基体圆盘...")
    myFeature = model.FeatureManager.FeatureExtrusion3(
        True,       # 单向
        False,      # 不反向
        False,      # 不等截面
        0,          # 终止条件类型1: 给定深度
        0,          # 终止条件类型2
        face_width, # 拉伸深度
        0,          # 拔模角度
        False, False, False, False,
        0, 0,       # 拔模方向
        False, False,
        False, False,
        False,
        False, False, False,
        0, 0,
        False       # 合并结果
    )

    if myFeature is None:
        print("警告：基体拉伸可能失败，但继续尝试...")
    else:
        print("基体圆盘拉伸完成")

    # ==================== 第二步：逐齿拉伸齿形 ====================
    print(f"正在创建 {z} 个齿...")

    for i in range(z):
        center_angle = i * tooth_angle
        half_base = (math.pi * m / 4) / root_r   # 齿根处半角
        half_tip = (math.pi * m / 4) / outer_r   # 齿顶处半角（略窄，模拟渐开线）

        # 齿形梯形四个角点
        bl_x = root_r * math.cos(center_angle - half_base)
        bl_y = root_r * math.sin(center_angle - half_base)
        tl_x = outer_r * math.cos(center_angle - half_tip)
        tl_y = outer_r * math.sin(center_angle - half_tip)
        tr_x = outer_r * math.cos(center_angle + half_tip)
        tr_y = outer_r * math.sin(center_angle + half_tip)
        br_x = root_r * math.cos(center_angle + half_base)
        br_y = root_r * math.sin(center_angle + half_base)

        # 获取基体上表面（拉伸方向 +Z）
        top_face = model.FeatureManager.GetFaceAtIndex(myFeature, 3)  # 上表面索引
        if top_face is None:
            # 尝试通过选择获取上表面
            model.Extension.SelectByID2("", "FACE",
                (root_r / 2) * math.cos(center_angle),
                (root_r / 2) * math.sin(center_angle),
                face_width, False, 0, None, 0)
            selMgr = model.SelectionManager
            top_face = selMgr.GetSelectedObject6(1, -1)

        if top_face is not None:
            top_face.Select2(False, 0)

        # 在上表面创建齿形草图
        sketchMgr.InsertSketch(True)

        # 绘制封闭梯形齿廓
        sketchMgr.CreateLine(bl_x, bl_y, 0, tl_x, tl_y, 0)    # 左侧齿廓
        sketchMgr.CreateLine(tl_x, tl_y, 0, tr_x, tr_y, 0)    # 齿顶
        sketchMgr.CreateLine(tr_x, tr_y, 0, br_x, br_y, 0)    # 右侧齿廓
        sketchMgr.CreateLine(br_x, br_y, 0, bl_x, bl_y, 0)    # 齿底

        sketchMgr.InsertSketch(True)

        # 拉伸齿
        tooth = model.FeatureManager.FeatureExtrusion3(
            True, False, False,
            0, 0,
            face_width,
            0,
            False, False, False, False,
            0, 0,
            False, False,
            False, False,
            True,    # 合并结果（与基体合并）
            False, False, False,
            0, 0,
            False
        )

        if (i + 1) % 5 == 0:
            print(f"  已完成 {i + 1}/{z} 个齿")

    print("所有齿形创建完成")

    # ==================== 第三步：拉伸切除中心孔（精确成型）====================
    print("正在精加工中心孔...")

    # 获取 Front Plane 并创建草图
    front_plane.Select2(False, 0)
    sketchMgr.InsertSketch(True)
    sketchMgr.CreateCircleByRadius(0, 0, 0, bore_dia / 2)
    sketchMgr.InsertSketch(True)

    # 拉伸切除贯穿
    model.FeatureManager.FeatureCut4(
        True, False, False,
        1, 1,     # 两侧贯穿
        0, 0,
        False, False, False, False,
        0, 0,
        False, False,
        False, False,
        False,
        False, False, False,
        0, 0,
        False
    )

    print("中心孔创建完成")

    # ==================== 视图操作 ====================
    model.ViewZoomtofit2()
    model.ForceRebuild3(True)

    print("=" * 50)
    print("齿轮零件绘制完成！")
    print(f"参数: 齿数={z}, 模数={m}mm, 压力角={alpha}°")
    print(f"外径={outer_r * 2}mm, 齿根圆径={root_r * 2}mm, 孔径={bore_dia}mm")
    print("=" * 50)


if __name__ == "__main__":
    main()
