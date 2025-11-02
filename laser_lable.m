% 選擇要標註的物件類型
fprintf('請選擇要標註的物件類型:\n');
fprintf('1 - 球 (Ball)\n');
fprintf('2 - 箱子 (Box)\n');
object_choice = input('請輸入選擇 (1 或 2): ');

if object_choice == 1
    % 處理球的資料
    segment_filename = 'segment_results_ball.mat';
    output_filename = 'ball_training_data.mat';
    object_name = '球';
elseif object_choice == 2
    % 處理箱子的資料
    segment_filename = 'segment_results_box.mat';
    output_filename = 'box_training_data.mat';
    object_name = '箱子';
else
    error('無效的選擇，請輸入 1 或 2');
end

% 讀取分段結果檔案
fprintf('正在載入 %s 的分段資料...\n', object_name);
segment_data = load(segment_filename);

% 從分段結果中獲取資料
all_segments = segment_data.all_segments;
scans = segment_data.scans;
scan_count = length(scans);

% 初始化分段標籤儲存
segment_labels = cell(scan_count, 1);

% 初始化所有分段標籤為0（背景）
for i = 1:scan_count
    segments = all_segments{i};
    segment_labels{i} = zeros(length(segments), 1);
end

% 設定選擇範圍的半徑（米）
selection_radius = 0.5;

fig = figure('Position', [200, 200, 800, 600]);

% 主循環
for i = 1:scan_count
    % 獲取當前分段和標籤
    segments = all_segments{i};
    current_segment_labels = segment_labels{i};
    
    % 確保標籤長度與分段數匹配
    if length(current_segment_labels) ~= length(segments)
        current_segment_labels = zeros(length(segments), 1);
        segment_labels{i} = current_segment_labels;
    end
    
    % 初始繪製
    redraw(i, scan_count, segments, current_segment_labels, object_name);
    
    % 等待用戶輸入
    while true
        % 使用 waitforbuttonpress 等待按鍵
        title(sprintf('%s - Scan %d/%d - Press r:select region | press c:reset | space:next', ...
            object_name, i, scan_count));
        waitforbuttonpress;
        key = get(gcf, 'CurrentCharacter');
        
        if key == ' ' % 空格鍵 - 下一頁
            break;
            
        elseif key == 'r' % r鍵 - 進入選擇模式
            % 顯示提示
            title(sprintf('點擊選擇 %s 區域 (按任意鍵取消)', object_name));
            
            % 準備 UserData，用於在回呼函數間傳遞資料
            ud.segments = segments;
            ud.current_segment_labels = current_segment_labels;
            ud.selection_radius = selection_radius;
            ud.boxHandle = []; % 用於儲存方形的 handle
            
            % 將 UserData 存入 figure
            set(fig, 'UserData', ud);
            
            % 設定回呼函數
            set(fig, 'WindowButtonMotionFcn', @moveSelectionBox);
            set(fig, 'WindowButtonDownFcn', @performSelection);
            
            % 等待用戶點擊 (performSelection 會呼叫 uiresume)
            try
                uiwait(fig);
            catch
                % 如果圖形被關閉，uiwait 會拋出錯誤
            end
            
            % 檢查 figure 是否仍然存在
            if ~ishandle(fig)
                break;
            end

            % 點擊完成後，清理回呼函數
            set(fig, 'WindowButtonMotionFcn', '');
            set(fig, 'WindowButtonDownFcn', '');

            if ~ishandle(fig)
                break;
            end

            % 從 UserData 取回更新後的標籤
            if isprop(fig, 'UserData')
                ud_after = get(fig, 'UserData');
                if isfield(ud_after, 'current_segment_labels')
                    current_segment_labels = ud_after.current_segment_labels;
                    segment_labels{i} = current_segment_labels;
                end
            end
            
            % 重新繪製
            redraw(i, scan_count, segments, current_segment_labels, object_name);
            
        elseif key == 'c' % c鍵 - 清空當前標籤
            current_segment_labels(:) = 0;
            segment_labels{i} = current_segment_labels;
            
            % 重新繪製
            redraw(i, scan_count, segments, current_segment_labels, object_name);
        end
    end
    
    % 檢查 figure 是否仍然有效
    if ~ishandle(fig)
        break;
    end
end

% 提取特徵並準備 Adaboost 訓練資料
fprintf('開始提取特徵並準備 Adaboost 訓練資料...\n');

features = [];
labels = [];

for i = 1:scan_count
    segments = all_segments{i};
    current_labels = segment_labels{i};
    
    for seg_idx = 1:length(segments)
        segment_points = segments{seg_idx};
        
        % 跳過點數太少的分段
        if size(segment_points, 1) < 3
            continue;
        end
        
        % 提取特徵
        seg_features = extract_segment_features(segment_points);
        
        % 轉換標籤格式: 0 → -1, 1 → +1
        if current_labels(seg_idx) == 1
            seg_label = +1;  % 正樣本
        else
            seg_label = -1;  % 負樣本
        end
        
        % 添加到訓練資料
        features = [features; seg_features];
        labels = [labels; seg_label];
    end
end

% 建立 Adaboost 訓練資料結構
training_data = struct();
training_data.features = features;
training_data.labels = labels;
training_data.feature_names = {'num_points', 'std_dev', 'width', 'circularity', 'radius'};
training_data.object_type = object_name;
training_data.total_samples = length(labels);
training_data.positive_samples = sum(labels == +1);
training_data.negative_samples = sum(labels == -1);

% 儲存 Adaboost 訓練資料
save(output_filename, 'training_data');
fprintf('\n%s 的 Adaboost 訓練資料已儲存至 %s\n', object_name, output_filename);

% 顯示統計
fprintf('\n=== %s 訓練資料統計 ===\n', object_name);
fprintf('總樣本數: %d\n', training_data.total_samples);
fprintf('正樣本 (%s): %d\n', object_name, training_data.positive_samples);
fprintf('負樣本 (背景): %d\n', training_data.negative_samples);
fprintf('正樣本比例: %.1f%%\n', (training_data.positive_samples / training_data.total_samples) * 100);

% 繪製當前掃描的函數
function redraw(scan_idx, scan_count, segments, current_segment_labels, object_name)
    fig = gcf;
    if ~ishandle(fig)
        return;
    end
    
    clf;
    
    % 繪製所有分段
    for seg_idx = 1:length(segments)
        segment_points = segments{seg_idx};
        
        if size(segment_points, 1) < 3
            continue;
        end
        
        % 根據標籤選擇顏色
        if current_segment_labels(seg_idx) == 1
            color = 'r';  % 物件分段 - 紅色
        else
            color = 'b';  % 背景分段 - 藍色
        end
        
        % 繪製分段點
        plot(segment_points(:,1), segment_points(:,2), '.', ...
             'Color', color, 'MarkerSize', 15);
        hold on;
    end
    
    % 繪製座標軸
    plot([-5, 5], [0, 0], 'k-', 'LineWidth', 0.5);
    plot([0, 0], [-5, 5], 'k-', 'LineWidth', 0.5);
    
    % 設定圖形屬性
    axis equal;
    xlim([-5, 5]);
    ylim([-5, 5]);
    grid on;
    
    title(sprintf('%s - Scan %d/%d - Press r:select region | press c:reset | space:next', ...
        object_name, scan_idx, scan_count));
    xlabel('x (m)');
    ylabel('y (m)');
    
    % 顯示統計
    object_segments = sum(current_segment_labels == 1);
    total_segments = length(segments);
    
    text(-4.5, 4.5, sprintf('%s segments: %d / %d', object_name, object_segments, total_segments), ...
         'FontSize', 12, 'BackgroundColor', 'white', 'VerticalAlignment', 'top');
    
    fprintf('Scan %d/%d - %s segments: %d, Background segments: %d\n', ...
            scan_idx, scan_count, object_name, object_segments, total_segments - object_segments);
end

% 滑鼠移動回呼函數：繪製選擇框
function moveSelectionBox(src, ~)
    if ~isprop(src, 'UserData') || isempty(get(src, 'UserData'))
        return;
    end
    
    ud = get(src, 'UserData');
    radius = ud.selection_radius;
    
    ax = gca;
    cp = get(ax, 'CurrentPoint');
    x = cp(1,1);
    y = cp(1,2);
    
    xLim = get(ax, 'XLim');
    yLim = get(ax, 'YLim');
    if x < xLim(1) || x > xLim(2) || y < yLim(1) || y > yLim(2)
        if isfield(ud, 'boxHandle') && ~isempty(ud.boxHandle) && all(ishandle(ud.boxHandle))
            delete(ud.boxHandle);
            ud.boxHandle = [];
            set(src, 'UserData', ud);
        end
        return;
    end
    
    x_min = x - radius;
    y_min = y - radius;
    width = 2 * radius;
    height = 2 * radius;
    
    if isfield(ud, 'boxHandle') && ~isempty(ud.boxHandle) && all(ishandle(ud.boxHandle))
        delete(ud.boxHandle);
    end
    
    hold on;
    ud.boxHandle = rectangle('Position', [x_min, y_min, width, height], ...
                             'EdgeColor', 'g', 'LineStyle', '--', 'LineWidth', 1.5);
    hold off;
    
    set(src, 'UserData', ud);
end

% 滑鼠點擊回呼函數：執行選擇
function performSelection(src, ~)
    if ~isprop(src, 'UserData') || isempty(get(src, 'UserData'))
        return;
    end

    ud = get(src, 'UserData');
    radius = ud.selection_radius;
    segments = ud.segments;
    current_segment_labels = ud.current_segment_labels;
    
    ax = gca;
    cp = get(ax, 'CurrentPoint');
    x_click = cp(1,1);
    y_click = cp(1,2);
    
    xLim = get(ax, 'XLim');
    yLim = get(ax, 'YLim');
    if x_click < xLim(1) || x_click > xLim(2) || y_click < yLim(1) || y_click > yLim(2)
        return;
    end
    
    if isfield(ud, 'boxHandle') && ~isempty(ud.boxHandle) && all(ishandle(ud.boxHandle))
        delete(ud.boxHandle);
        ud.boxHandle = [];
    end
    
    x_min = x_click - radius;
    x_max = x_click + radius;
    y_min = y_click - radius;
    y_max = y_click + radius;
    
    segments_in_region = [];
    for seg_idx = 1:length(segments)
        segment_points = segments{seg_idx};
        centroid = mean(segment_points, 1);
        
        if centroid(1) >= x_min && centroid(1) <= x_max && ...
           centroid(2) >= y_min && centroid(2) <= y_max
            segments_in_region = [segments_in_region, seg_idx];
        end
    end
    
    if ~isempty(segments_in_region)
        current_segment_labels(segments_in_region) = 1;
    end
    
    ud.current_segment_labels = current_segment_labels;
    set(src, 'UserData', ud);
    
    uiresume(src);
end

% 特徵提取函數
function features = extract_segment_features(points)
    % 特徵1: 點的數量
    num_points = size(points, 1);
    
    % 特徵2: 標準差
    centroid = mean(points, 1);
    distances = sqrt(sum((points - centroid).^2, 2));
    std_dev = std(distances);
    
    % 特徵3: 寬度 (x方向範圍)
    width = max(points(:,1)) - min(points(:,1));
    
    % 特徵4 & 5: 圓形度和半徑
    [circularity, radius] = calculate_circularity(points);
    
    features = [num_points, std_dev, width, circularity, radius];
end

function [circularity, radius] = calculate_circularity(points)
    % 簡單的圓形擬合
    n = size(points, 1);
    x = points(:,1);
    y = points(:,2);
    
    % 計算質心
    xc = mean(x);
    yc = mean(y);
    
    % 計算平均半徑
    distances = sqrt((x - xc).^2 + (y - yc).^2);
    radius = mean(distances);
    
    % 圓形度誤差
    circularity = sum((distances - radius).^2) / n;
end