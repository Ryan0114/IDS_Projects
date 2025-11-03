% 設定檔案名稱
filename = 'laser_data_ball.dat';  % 請替換為你的 .dat 檔案名稱

% 讀取所有行
fid = fopen(filename, 'r');
if fid == -1
    error('無法開啟檔案: %s', filename);
end

% 讀取所有掃描幀
scans = {};
while ~feof(fid)
    line = fgetl(fid);
    if isempty(line)
        continue;
    end
    % 將行資料轉換為數值陣列
    scan_data = str2num(line); %#ok<ST2NM>
    if mod(length(scan_data), 2) == 0  % 確保是偶數個數值（角度、距離配對）
        scans{end+1} = scan_data;
    end
end
fclose(fid);

% 分段閾值 (5cm)
segment_threshold = 0.05;

% 檢查是否有掃描資料
if isempty(scans)
    error('沒有讀取到有效的掃描資料');
end

scan_count = length(scans);

% 初始化儲存結構
all_segments = cell(scan_count, 1);  % 儲存每個掃描的分段結果

% 調整動畫時間
frame_duration = 30 / scan_count;

figure;
start_time = tic;
for i = 1:scan_count
    clf;
    
    row_data = scans{i};
    angles = row_data(1:2:end);   % 奇數索引：角度
    distances = row_data(2:2:end); % 偶數索引：距離
    
    % 移除無效資料點
    valid_idx = ~isnan(distances) & ~isinf(distances) & distances > 0;
    angles = angles(valid_idx);
    distances = distances(valid_idx);
    
    if isempty(angles)
        all_segments{i} = {};
        continue;
    end
    
    % 轉換為笛卡爾座標
    theta = deg2rad(angles);
    x = distances .* cos(theta);
    y = distances .* sin(theta);
    
    % 分段處理
    segments = segmentLaserData(x, y, segment_threshold);
    
    % 儲存分段結果
    all_segments{i} = segments;
    
    % 對每個段落用不同顏色繪製
    colors = lines(length(segments));
    
    total_points_in_segments = 0;
    
    for seg_idx = 1:length(segments)
        segment_points = segments{seg_idx};
        
        if size(segment_points, 1) < 3
            continue;  % 跳過點數太少的段落
        end
        
        % 繪製段落點
        plot(segment_points(:,1), segment_points(:,2), '.', ...
             'Color', colors(seg_idx, :), 'MarkerSize', 15);
        hold on;
        
        total_points_in_segments = total_points_in_segments + size(segment_points, 1);
    end
    
    % 設定圖形屬性
    axis equal;
    xlim([-5, 5]);
    set(gca, 'XTick', -5:1:5);
    ylim([-5, 5]);
    set(gca, 'YTick', -5:1:5);

    xlabel('x (m)');
    ylabel('y (m)');
    title(sprintf('Scan %d/%d - 雷射掃描資料 (共 %d 個段落)', i, scan_count, length(segments)));
    grid on;
    
    % 顯示統計資訊
    total_points = length(x);
    total_segments = length(segments);
    
    stats_text = sprintf('總點數: %d\n總段落數: %d', total_points, total_segments);
    text(-4.5, 4.5, stats_text, 'FontSize', 10, 'BackgroundColor', 'white', ...
         'VerticalAlignment', 'top');
    
    drawnow;

    % 時間控制
    elapsed_time = toc(start_time);
    expected_time = i * frame_duration;
    pause_time = max(0.001, expected_time - elapsed_time);
    if pause_time > 0.001
        pause(pause_time);
    end
end

% 計算基本統計資訊
total_segments_all_scans = 0;
segment_points_count = [];

for i = 1:scan_count
    segments = all_segments{i};
    total_segments_all_scans = total_segments_all_scans + length(segments);
    for j = 1:length(segments)
        segment_points_count = [segment_points_count; size(segments{j}, 1)];
    end
end

% 顯示統計
fprintf('\n=== 分段結果統計 ===\n');
fprintf('總掃描幀數: %d\n', scan_count);
fprintf('總段落數: %d\n', total_segments_all_scans);
fprintf('平均每段點數: %.2f\n', mean(segment_points_count));
fprintf('最小段落點數: %d\n', min(segment_points_count));
fprintf('最大段落點數: %d\n', max(segment_points_count));

% 僅儲存標籤程式需要的資料 - 單一輸出檔案
output_filename = 'segment_results_ball.mat';
save(output_filename, 'all_segments', 'scans');
fprintf('分段結果已儲存至: %s\n', output_filename);
fprintf('檔案包含變數: all_segments, scans \n');

% 分段函數
function segments = segmentLaserData(x, y, max_gap)
    segments = {};
    current_segment = [];
    
    if isempty(x)
        return;
    end
    
    % 從第一個點開始
    current_segment = [x(1), y(1)];
    
    for i = 2:length(x)
        % 計算當前點與上一個點的距離
        prev_point = [x(i-1), y(i-1)];
        curr_point = [x(i), y(i)];
        distance = norm(curr_point - prev_point);
        
        if distance <= max_gap
            % 距離小於5cm，加入當前段落
            current_segment = [current_segment; curr_point];
        else
            % 距離大於5cm，結束當前段落，開始新段落
            if size(current_segment, 1) >= 3  % 至少3個點才視為有效段落
                segments{end+1} = current_segment;
            end
            current_segment = curr_point;
        end
    end
    
    % 加入最後一個段落
    if size(current_segment, 1) >= 3
        segments{end+1} = current_segment;
    end
end