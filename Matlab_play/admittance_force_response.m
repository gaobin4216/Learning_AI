%% 导纳控制力响应仿真 - 7自由度机械臂实景仿真
% 功能：机械臂受0→100N外力后撤力，观察导纳控制响应
% 日期：2026-05-31

clear; clc; close all;

%% 1. 机器人DH参数 (7-DOF)
dh_params = [
    0,      -pi/2,  0.340,  0;   % 关节1
    0,       pi/2,  0,      0;   % 关节2
    0,      -pi/2,  0.400,  0;   % 关节3
    0,       pi/2,  0,      0;   % 关节4
    0,      -pi/2,  0.400,  0;   % 关节5
    0,       pi/2,  0,      0;   % 关节6
    0,       0,     0.126,  0;   % 关节7
];
n_joints = 7;

%% 2. 导纳控制参数
M_adm = 10;      % 虚拟惯性 (kg)
B_adm = 200;     % 虚拟阻尼 (N·s/m)
K_adm = 500;     % 虚拟刚度 (N/m)

%% 3. 仿真参数
dt = 0.001;
T_total = 4.0;
t = 0:dt:T_total;
N = length(t);

% 初始关节角度
q0 = [0; pi/6; 0; -pi/3; 0; pi/4; 0];
q = q0;

%% 4. 外部力设计 (沿X轴正方向)
F_ext = zeros(1, N);
F_dir = [1; 0; 0];  % 力方向：X轴正方向
for i = 1:N
    if t(i) <= 1.0
        F_ext(i) = 100 * t(i);      % 0→100N
    elseif t(i) <= 2.0
        F_ext(i) = 100;              % 保持100N
    else
        F_ext(i) = 0;                % 撤力
    end
end

%% 5. 导纳控制仿真
x_adm = 0;      % 导纳修正位移
dx_adm = 0;     % 导纳修正速度

% 存储数据
q_history = zeros(n_joints, N);
x_ee_history = zeros(3, N);
x_adm_history = zeros(1, N);

fprintf('开始仿真...\n');

for i = 1:N
    % 正运动学
    T = forward_kinematics(q, dh_params);
    x_ee = T(1:3, 4);

    % 导纳控制器: M*ddx + B*dx + K*x = F_ext
    ddx_adm = (F_ext(i) - B_adm * dx_adm - K_adm * x_adm) / M_adm;
    dx_adm = dx_adm + ddx_adm * dt;
    x_adm = x_adm + dx_adm * dt;

    % 计算雅可比
    J = compute_jacobian(q, dh_params);

    % 逆运动学：通过导纳修正调整关节角度
    % 期望末端位置 = 初始位置 + 导纳修正 (沿X轴方向)
    x_ee_init = forward_kinematics(q0, dh_params);
    x_desired = x_ee_init(1:3, 4) + F_dir * x_adm;

    % 简化的逆运动学 (雅可比迭代法)
    x_error = x_desired - x_ee;
    dq = pinv(J(1:3, :)) * x_error * 0.5;  % 增益0.5
    q = q + dq;

    % 存储
    q_history(:, i) = q;
    x_ee_history(:, i) = x_ee;
    x_adm_history(i) = x_adm;
end

fprintf('仿真完成!\n');

%% 6. 实时3D动画展示（可拖拽旋转）
fprintf('正在展示实时动画（可鼠标拖拽旋转3D视图）...\n');

fig = figure('Position', [100, 100, 1400, 700], 'Name', '7自由度机械臂导纳控制实时动画');

% ===== 左侧子图：3D机械臂（初始化静态对象） =====
subplot(1, 2, 1);

% 初始关节位置
joint_pos = calc_joint_positions(q_history(:,1), dh_params);
ee = joint_pos(:, end);

hold on;
% 地面（静态）
fill3([-0.5, 0.8, 0.8, -0.5], [-0.5, -0.5, 0.5, 0.5], ...
      [0, 0, 0, 0], [0.9, 0.9, 0.9], 'FaceAlpha', 0.3);
% 基座（静态）
fill3([-0.1, 0.1, 0.1, -0.1], [-0.1, -0.1, 0.1, 0.1], ...
      [0, 0, 0.05, 0.05], [0.3, 0.3, 0.3]);
plot3(0, 0, 0, 'ks', 'MarkerSize', 15, 'MarkerFaceColor', 'k');
% 机械臂连杆
h_arm = plot3(joint_pos(1,:), joint_pos(2,:), joint_pos(3,:), ...
      'b-o', 'LineWidth', 4, 'MarkerSize', 10, 'MarkerFaceColor', [0.2, 0.4, 1]);
% 末端执行器
h_ee = plot3(ee(1), ee(2), ee(3), 'ro', 'MarkerSize', 12, 'MarkerFaceColor', 'r');
% 外力箭头（初始不可见）
h_arrow = quiver3(ee(1), ee(2), ee(3), 0, 0, 0, ...
          'r', 'LineWidth', 3, 'MaxHeadSize', 0.5, 'AutoScale', 'off', 'Visible', 'off');
h_ftext = text(ee(1)+0.15, ee(2), ee(3)+0.05, '', ...
          'FontSize', 12, 'FontWeight', 'bold', 'Color', 'r', 'Visible', 'off');
% 轨迹
h_traj = plot3(x_ee_history(1,1), x_ee_history(2,1), x_ee_history(3,1), ...
      'g-', 'LineWidth', 1.5);
% 时间和阶段文本
h_ttext = text(0.02, 0.02, '', 'Units', 'normalized', 'FontSize', 12, 'FontWeight', 'bold');
h_ptext = text(0.02, 0.96, '', 'Units', 'normalized', 'FontSize', 11, 'FontWeight', 'bold');

xlabel('X (m)'); ylabel('Y (m)'); zlabel('Z (m)');
title('7自由度机械臂导纳控制', 'FontSize', 13, 'FontWeight', 'bold');
grid on; axis equal;
xlim([-0.5, 0.8]); ylim([-0.5, 0.5]); zlim([-0.1, 1.2]);
view(35, 25);
rotate3d on;

% ===== 右侧子图：响应曲线（初始化静态对象） =====
subplot(1, 2, 2);
yyaxis left;
h_disp = plot(t(1), x_adm_history(1)*1000, 'b-', 'LineWidth', 2);
ylabel('导纳修正位移 (mm)'); ylim([-5, 220]);
hold on;
yline(200, 'b--', '理论稳态', 'LineWidth', 1);

yyaxis right;
h_force = plot(t(1), F_ext(1), 'r-', 'LineWidth', 2);
ylabel('外部力 (N)'); ylim([-5, 120]);

xlabel('时间 (s)'); xlim([0, T_total]);
title('导纳控制响应曲线', 'FontSize', 13, 'FontWeight', 'bold');
legend('位移响应', '外部力', 'Location', 'northwest'); grid on;
xline(1.0, 'k--', 'LineWidth', 1);
xline(2.0, 'k--', 'LineWidth', 1);

h_sgtitle = sgtitle('', 'FontSize', 14, 'FontWeight', 'bold');

% ===== 动画主循环（仅更新数据，不重建对象） =====
frame_step = 40;  % 总约100帧，快速流畅
for i = frame_step:frame_step:N
    % 更新机械臂
    joint_pos = calc_joint_positions(q_history(:,i), dh_params);
    set(h_arm, 'XData', joint_pos(1,:), 'YData', joint_pos(2,:), 'ZData', joint_pos(3,:));
    ee = joint_pos(:, end);
    set(h_ee, 'XData', ee(1), 'YData', ee(2), 'ZData', ee(3));

    % 更新外力箭头
    if F_ext(i) > 0
        arrow_len = F_ext(i) / 500;
        set(h_arrow, 'XData', ee(1), 'YData', ee(2), 'ZData', ee(3), ...
            'UData', arrow_len, 'VData', 0, 'WData', 0, 'Visible', 'on');
        set(h_ftext, 'Position', [ee(1)+0.15, ee(2), ee(3)+0.05], ...
            'String', sprintf('F=%.0fN', F_ext(i)), 'Visible', 'on');
    else
        set(h_arrow, 'Visible', 'off');
        set(h_ftext, 'Visible', 'off');
    end

    % 更新轨迹
    set(h_traj, 'XData', x_ee_history(1, 1:i), 'YData', x_ee_history(2, 1:i), ...
        'ZData', x_ee_history(3, 1:i));

    % 更新文本
    set(h_ttext, 'String', sprintf('t = %.2f s', t(i)));
    if t(i) <= 1.0
        set(h_ptext, 'String', '施力阶段: 0->100N', 'Color', [1, 0.5, 0]);
    elseif t(i) <= 2.0
        set(h_ptext, 'String', '保持阶段: 100N', 'Color', [1, 0, 0]);
    else
        set(h_ptext, 'String', '恢复阶段: 力已撤走', 'Color', [0, 0.7, 0]);
    end

    % 更新响应曲线
    set(h_disp, 'XData', t(1:i), 'YData', x_adm_history(1:i)*1000);
    set(h_force, 'XData', t(1:i), 'YData', F_ext(1:i));
    set(h_sgtitle, 'String', sprintf('导纳控制仿真  t = %.2f s  |  M=%dkg, B=%dN·s/m, K=%dN/m', ...
            t(i), M_adm, B_adm, K_adm));

    drawnow;
end

fprintf('动画展示完成！现在可以拖拽旋转3D视图查看最终姿态。\n');
fprintf('按任意键关闭窗口...\n');
pause;

%% ==================== 辅助函数 ====================

function T = forward_kinematics(q, dh_params)
    T = eye(4);
    for i = 1:size(dh_params, 1)
        a = dh_params(i, 1);
        alpha = dh_params(i, 2);
        d = dh_params(i, 3);
        theta = q(i) + dh_params(i, 4);
        T = T * dh_transform(a, alpha, d, theta);
    end
end

function T = dh_transform(a, alpha, d, theta)
    T = [cos(theta), -sin(theta)*cos(alpha),  sin(theta)*sin(alpha), a*cos(theta);
         sin(theta),  cos(theta)*cos(alpha), -cos(theta)*sin(alpha), a*sin(theta);
         0,           sin(alpha),             cos(alpha),            d;
         0,           0,                      0,                     1];
end

function J = compute_jacobian(q, dh_params)
    n = length(q);
    J = zeros(6, n);
    T = eye(4);
    T_all = zeros(4, 4, n+1);
    T_all(:,:,1) = T;

    for i = 1:n
        a = dh_params(i, 1);
        alpha = dh_params(i, 2);
        d = dh_params(i, 3);
        theta = q(i) + dh_params(i, 4);
        T = T * dh_transform(a, alpha, d, theta);
        T_all(:,:,i+1) = T;
    end

    p_e = T_all(1:3, 4, end);
    for i = 1:n
        z_i = T_all(1:3, 3, i);
        p_i = T_all(1:3, 4, i);
        J(1:3, i) = cross(z_i, p_e - p_i);
        J(4:6, i) = z_i;
    end
end

function joint_pos = calc_joint_positions(q, dh_params)
    n = length(q);
    joint_pos = zeros(3, n + 1);
    T_cum = eye(4);
    for i = 1:n
        a = dh_params(i, 1);
        alpha = dh_params(i, 2);
        d = dh_params(i, 3);
        theta = q(i) + dh_params(i, 4);
        T_cum = T_cum * dh_transform(a, alpha, d, theta);
        joint_pos(:, i+1) = T_cum(1:3, 4);
    end
end
