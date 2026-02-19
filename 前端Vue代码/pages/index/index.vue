<template>
	<view class="container">
		<!-- 顶部天气时钟区域 -->
		<view class="header">
			<view class="title-bar">
				<text class="app-title">智能环境监测与安防系统</text>
			</view>
			<view class="weather-clock-box">
				<view class="clock-section">
					<text class="time">{{ timeStr }}</text>
					<text class="date">{{ dateStr }}</text>
				</view>
				<view class="weather-section" v-if="weather">
					<text class="weather-temp">{{ weather.temperature }}°C</text>
					<text class="weather-status">{{ city }} | {{ weatherDesc }}</text>
				</view>
				<view class="weather-section" v-else>
					<text class="weather-status">加载天气...</text>
				</view>
			</view>
		</view>

		<!-- 多巴胺风格卡片网格 -->
		<view class="grid-container">
			<!-- 温度卡片 -->
			<view class="card temp-card">
				<view class="card-header">
					<text class="card-label">温度</text>
					<text class="card-icon">🌡️</text>
				</view>
				<view class="card-body">
					<text class="card-value">{{ temp || '--' }}</text>
					<text class="card-unit">°C</text>
				</view>
			</view>

			<!-- 湿度卡片 -->
			<view class="card humi-card">
				<view class="card-header">
					<text class="card-label">湿度</text>
					<text class="card-icon">💧</text>
				</view>
				<view class="card-body">
					<text class="card-value">{{ humi || '--' }}</text>
					<text class="card-unit">%</text>
				</view>
			</view>

			<!-- 光照强度卡片 -->
			<view class="card light-lux-card">
				<view class="card-header">
					<text class="card-label">光照</text>
					<text class="card-icon">☀️</text>
				</view>
				<view class="card-body">
					<text class="card-value">{{ light || '--' }}</text>
					<text class="card-unit">Lux</text>
				</view>
			</view>

			<!-- 距离卡片 -->
			<view class="card distance-card">
				<view class="card-header">
					<text class="card-label">距离</text>
					<text class="card-icon">📏</text>
				</view>
				<view class="card-body">
					<text class="card-value">{{ distance || '--' }}</text>
					<text class="card-unit">cm</text>
				</view>
			</view>

			<!-- 台灯控制卡片 -->
			<view class="card light-card">
				<view class="card-header">
					<text class="card-label">台灯</text>
					<text class="card-icon">💡</text>
				</view>
				<view class="card-body row-body">
					<text class="status-text">{{ led ? '开启' : '关闭' }}</text>
					<switch color="#FFD700" :checked="led" @change="onLedSwitch" style="transform:scale(0.8)"/>
				</view>
			</view>

			<!-- 蜂鸣器控制卡片 -->
			<view class="card buffer-card">
				<view class="card-header">
					<text class="card-label">蜂鸣器</text>
					<text class="card-icon">🔔</text>
				</view>
				<view class="card-body row-body">
					<text class="status-text">{{ buffer ? '开启' : '关闭' }}</text>
					<switch color="#FF6B6B" :checked="buffer" @change="onBufferSwitch" style="transform:scale(0.8)"/>
				</view>
			</view>

			<!-- 电机控制卡片 -->
			<view class="card motor-card">
				<view class="card-header">
					<text class="card-label">电机</text>
					<text class="card-icon">⚙️</text>
				</view>
				<view class="card-body row-body">
					<text class="status-text">{{ motor_status ? '开启' : '关闭' }}</text>
					<switch color="#9D50BB" :checked="motor_status" @change="onMotorSwitch" style="transform:scale(0.8)"/>
				</view>
			</view>

			<!-- 用户信息卡片 -->
			<view class="card info-card">
				<view class="card-body centered">
					<text class="user-name">54faye</text>
					<text class="user-role">IoT Admin</text>
					<text class="update-time" v-if="lastUpdate">更新于: {{ lastUpdate }}</text>
				</view>
			</view>
		</view>
	</view>
</template>

<script>
	const {
		createCommonToken
	} = require('@/key.js')
	export default {
		data() {
			return {
				temp: '',
				humi: '',
				led: true,
				buffer: false, // 蜂鸣器
				motor_status: false, // 电机
				distance: '', // 距离
				light: '', // 光照
				token: '',
				timeStr: '',
				dateStr: '',
				weather: null,
				weatherDesc: '',
				city: '北京', // 默认城市，可结合 uni.getLocation 动态获取
				timer: null,
				dataTimer: null, // 数据刷新定时器
				lastUpdate: '' // 最后更新时间
			}
		},
		onLoad() {
			// 连接鉴权信息
			const params = {
				access_key: '7ROl6flhl+mJZ5dL7zclH8nxfXfL83sDqz/Lo/lX2ys=',
				version: '2022-05-01',
				productid: '8OCvDyLab8',
			}
			this.token = createCommonToken(params);
			
			// 启动时钟
			this.startClock();
			// 获取天气
			this.fetchWeather();
		},
		onShow() {
			this.fetchDevData();
			// 开启自动刷新，每3秒刷新一次
			this.startDataRefresh();
		},
		onHide() {
			// 页面隐藏时停止刷新，节省资源
			this.stopDataRefresh();
		},
		onUnload() {
			if (this.timer) {
				clearInterval(this.timer);
			}
			this.stopDataRefresh();
		},
		methods: {
			startDataRefresh() {
				this.stopDataRefresh(); // 防止重复开启
				this.dataTimer = setInterval(() => {
					this.fetchDevData();
				}, 3000); // 3秒刷新一次
			},
			stopDataRefresh() {
				if (this.dataTimer) {
					clearInterval(this.dataTimer);
					this.dataTimer = null;
				}
			},
			startClock() {
				const update = () => {
					const now = new Date();
					this.timeStr = now.toLocaleTimeString('zh-CN', { hour12: false });
					const days = ['周日', '周一', '周二', '周三', '周四', '周五', '周六'];
					this.dateStr = `${now.getMonth() + 1}月${now.getDate()}日 ${days[now.getDay()]}`;
				};
				update();
				this.timer = setInterval(update, 1000);
			},
			fetchWeather() {
				// 尝试获取当前位置
				uni.getLocation({
					type: 'wgs84',
					success: (res) => {
						console.log('定位成功:', res);
						// 获取天气
						this.getWeatherByLocation(res.latitude, res.longitude);
						// 获取城市名称
						this.getCityName(res.latitude, res.longitude);
					},
					fail: (err) => {
						console.log('定位失败或权限未开启，使用默认位置(北京)', err);
						this.city = '北京';
						this.getWeatherByLocation(39.9042, 116.4074);
					}
				});
			},
			getCityName(latitude, longitude) {
				uni.request({
					// 使用免费的反向地理编码 API获取城市名称
					url: `https://api.bigdatacloud.net/data/reverse-geocode-client?latitude=${latitude}&longitude=${longitude}&localityLanguage=zh`,
					success: (res) => {
						console.log('位置信息:', res.data);
						if (res.data) {
							// 优先取城市名，其次取行政区名
							this.city = res.data.city || res.data.locality || '本地';
						}
					},
					fail: () => {
						this.city = '本地';
					}
				});
			},
			getWeatherByLocation(latitude, longitude) {
				uni.request({
					url: `https://api.open-meteo.com/v1/forecast?latitude=${latitude}&longitude=${longitude}&current_weather=true&timezone=Asia%2FShanghai`,
					success: (res) => {
						if (res.data && res.data.current_weather) {
							this.weather = res.data.current_weather;
							this.weatherDesc = this.getWeatherDesc(this.weather.weathercode);
						}
					},
					fail: (err) => {
						console.error('天气获取失败', err);
					}
				});
			},
			getWeatherDesc(code) {
				const map = {
					0: '晴',
					1: '晴', 2: '多云', 3: '阴',
					45: '雾', 48: '雾',
					51: '小雨', 53: '中雨', 55: '大雨',
					61: '小雨', 63: '中雨', 65: '大雨',
					71: '小雪', 73: '中雪', 75: '大雪',
					80: '阵雨', 81: '阵雨', 82: '暴雨',
					95: '雷雨', 96: '雷雨', 99: '雷雨'
				};
				return map[code] || '未知';
			},
			fetchDevData() {
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
					method: 'GET',
					data: {
						product_id: '8OCvDyLab8',
						device_name: 'w1',
						_t: Date.now() // 添加时间戳，防止请求缓存
					},
					header: {
						'authorization': this.token //自定义请求头信息
					},
					success: (res) => {
						console.log(res.data);
						if (res.data.code === 0 && res.data.data) {
							this.lastUpdate = new Date().toLocaleTimeString();
							// 遍历返回的数据数组，根据 identifier 匹配
							res.data.data.forEach(item => {
								switch (item.identifier) {
									case 'temp':
										this.temp = item.value;
										break;
									case 'humi':
										this.humi = item.value;
										break;
									case 'led':
										this.led = String(item.value) === 'true';
										break;
									case 'buffer':
										this.buffer = String(item.value) === 'true';
										break;
									case 'motor_status':
										this.motor_status = String(item.value) === 'true';
										break;
									case 'distance':
										this.distance = item.value;
										break;
									case 'light':
										this.light = item.value;
										break;
								}
							});
						}
					}
				});
			},
			onLedSwitch(event) {
				let value = event.detail.value;
				this.led = value;
				this.sendDeviceCmd('led', value);
			},
			onBufferSwitch(event) {
				let value = event.detail.value;
				this.buffer = value;
				this.sendDeviceCmd('buffer', value);
			},
			onMotorSwitch(event) {
				let value = event.detail.value;
				this.motor_status = value;
				this.sendDeviceCmd('motor_status', value);
			},
			sendDeviceCmd(key, value) {
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: '8OCvDyLab8',
						device_name: 'w1',
						params: {
							[key]: value
						}
					},
					header: {
						'authorization': this.token //自定义请求头信息
					},
					success: () => {
						console.log(`${key}: ${value ? 'on' : 'off'} success!`);
					},
					fail: () => {
						uni.showToast({
							title: '控制失败',
							icon: 'none'
						});
					}
				});
			}
		}
	}
</script>

<style lang="scss">
	page {
		background-color: #f0f2f5;
		font-family: system-ui, -apple-system, sans-serif;
	}

	.container {
		padding: 30rpx;
		min-height: 100vh;
		box-sizing: border-box;
	}

	/* 头部样式 */
	.header {
		background: linear-gradient(120deg, #89f7fe 0%, #66a6ff 100%);
		border-radius: 30rpx;
		padding: 40rpx;
		margin-bottom: 40rpx;
		color: #fff;
		box-shadow: 0 10rpx 30rpx rgba(102, 166, 255, 0.3);
		position: relative;
		overflow: hidden;
	}
	
	.header::after {
		content: '';
		position: absolute;
		top: -50%;
		right: -20%;
		width: 300rpx;
		height: 300rpx;
		background: rgba(255,255,255,0.1);
		border-radius: 50%;
	}

	.app-title {
		font-size: 32rpx;
		font-weight: bold;
		opacity: 0.9;
		display: block;
		margin-bottom: 30rpx;
		text-align: center;
	}

	.weather-clock-box {
		display: flex;
		justify-content: space-between;
		align-items: center;
	}

	.clock-section {
		display: flex;
		flex-direction: column;
	}

	.time {
		font-size: 64rpx;
		font-weight: bold;
		line-height: 1;
		margin-bottom: 10rpx;
		text-shadow: 0 2rpx 4rpx rgba(0,0,0,0.1);
	}

	.date {
		font-size: 28rpx;
		opacity: 0.9;
	}

	.weather-section {
		display: flex;
		flex-direction: column;
		align-items: flex-end;
	}

	.weather-temp {
		font-size: 56rpx;
		font-weight: bold;
	}

	.weather-status {
		font-size: 24rpx;
		background: rgba(255,255,255,0.2);
		padding: 4rpx 16rpx;
		border-radius: 20rpx;
		margin-top: 8rpx;
	}

	/* 网格布局 */
	.grid-container {
		display: flex;
		flex-wrap: wrap;
		justify-content: space-between;
	}

	.card {
		width: 48%;
		background: #fff;
		border-radius: 30rpx;
		padding: 30rpx;
		margin-bottom: 30rpx;
		box-sizing: border-box;
		display: flex;
		flex-direction: column;
		justify-content: space-between;
		box-shadow: 0 8rpx 20rpx rgba(0,0,0,0.03);
		transition: all 0.3s ease;
		position: relative;
		overflow: hidden;
		height: 240rpx;
	}
	
	.card:active {
		transform: scale(0.98);
	}

	.card-header {
		display: flex;
		justify-content: space-between;
		align-items: flex-start;
		margin-bottom: 20rpx;
	}

	.card-label {
		font-size: 28rpx;
		color: #666;
		font-weight: 500;
	}

	.card-icon {
		font-size: 40rpx;
	}

	.card-body {
		display: flex;
		align-items: baseline;
	}
	
	.row-body {
		align-items: center;
		justify-content: space-between;
		width: 100%;
	}

	.card-value {
		font-size: 56rpx;
		font-weight: bold;
		color: #333;
	}

	.card-unit {
		font-size: 28rpx;
		color: #999;
		margin-left: 8rpx;
	}

	/* 多巴胺配色风格 */
	
	/* 温度 - 暖色调 */
	.temp-card {
		background: linear-gradient(145deg, #ffffff 0%, #fff0f0 100%);
		border: 2rpx solid rgba(255, 154, 158, 0.2);
	}
	.temp-card .card-value { color: #ff6b6b; }
	.temp-card .card-icon { 
		background: #ffe2e2; 
		padding: 10rpx; 
		border-radius: 50%;
		font-size: 32rpx;
	}

	/* 湿度 - 冷色调 */
	.humi-card {
		background: linear-gradient(145deg, #ffffff 0%, #f0f8ff 100%);
		border: 2rpx solid rgba(79, 172, 254, 0.2);
	}
	.humi-card .card-value { color: #4facfe; }
	.humi-card .card-icon { 
		background: #e0f2ff; 
		padding: 10rpx; 
		border-radius: 50%;
		font-size: 32rpx;
	}

	/* 光照 - 阳光橙 */
	.light-lux-card {
		background: linear-gradient(145deg, #ffffff 0%, #fffdf0 100%);
		border: 2rpx solid rgba(255, 165, 2, 0.2);
	}
	.light-lux-card .card-value { color: #fece2f; }
	.light-lux-card .card-icon {
		background: #fff4d1;
		padding: 10rpx;
		border-radius: 50%;
		font-size: 32rpx;
	}

	/* 距离 - 清新绿 */
	.distance-card {
		background: linear-gradient(145deg, #ffffff 0%, #f0fff4 100%);
		border: 2rpx solid rgba(46, 204, 113, 0.2);
	}
	.distance-card .card-value { color: #2ecc71; }
	.distance-card .card-icon {
		background: #e0f9e9;
		padding: 10rpx;
		border-radius: 50%;
		font-size: 32rpx;
	}

	/* 台灯 - 亮黄色 */
	.light-card {
		background: linear-gradient(145deg, #ffffff 0%, #ffffec 100%);
		border: 2rpx solid rgba(255, 210, 0, 0.2);
	}
	.light-card .card-icon { 
		background: #fff8cc; 
		padding: 10rpx; 
		border-radius: 50%;
		font-size: 32rpx;
	}
	.status-text {
		font-size: 28rpx;
		color: #888;
	}

	/* 蜂鸣器 - 警示红 */
	.buffer-card {
		background: linear-gradient(145deg, #ffffff 0%, #fff5f5 100%);
		border: 2rpx solid rgba(255, 107, 107, 0.2);
	}
	.buffer-card .card-icon {
		background: #ffe3e3;
		padding: 10rpx;
		border-radius: 50%;
		font-size: 32rpx;
	}

	/* 电机 - 机械紫 */
	.motor-card {
		background: linear-gradient(145deg, #ffffff 0%, #f8f0ff 100%);
		border: 2rpx solid rgba(157, 80, 187, 0.2);
	}
	.motor-card .card-icon {
		background: #f3e5f5;
		padding: 10rpx;
		border-radius: 50%;
		font-size: 32rpx;
	}

	/* 用户信息 - 渐变色 */
	.info-card {
		background: linear-gradient(135deg, #a18cd1 0%, #fbc2eb 100%);
		border: none;
		color: white;
	}
	
	.info-card .centered {
		flex-direction: column;
		align-items: center;
		justify-content: center;
		width: 100%;
		height: 100%;
	}
	
	.user-name {
		font-size: 48rpx;
		font-weight: bold;
		margin-bottom: 10rpx;
		text-shadow: 0 2rpx 4rpx rgba(0,0,0,0.1);
	}
	
	.user-role {
		font-size: 24rpx;
		opacity: 0.8;
		background: rgba(255,255,255,0.2);
		padding: 4rpx 16rpx;
		border-radius: 20rpx;
	}
	
	.update-time {
		font-size: 20rpx;
		margin-top: 10rpx;
		opacity: 0.7;
	}
</style>