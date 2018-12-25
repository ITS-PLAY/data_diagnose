#include <string>
#include <cstring>
#include <iostream>
#include <vector>          //定义vector元素
using namespace std;             //解决string类型不能被识别的问题


#define Time_Interval_LATENCY 5       //延迟判断的时间间隔
#define Time_Limit_Value 2            //延迟的时间阈值
#define data_latency_ratio_Limit 0.05  //数据延迟的比率阈值

#define Time_Interval_MISS 15        //数据缺失判断的时间间隔
#define Time_Interval_ANOMALY 15     //数据异常判断的时间间隔
#define Predictor_Data_Num 1         //数据异常中预测未来若个时段流量的个数
#define Gaussian_Num 3               //混合高斯分布的分布个数
#define Confidence_Interval 3        //置信区间中标准差的倍数
#define PI acos(-1.0)        


struct Flow_Diagram{            //存储流量值和对应的频率
	vector<int> FLOW_VALUE;
	vector<float> FLOW_FREQUENCY;
};


struct EM_Output{
	float EM_ave;
	float EM_sd;
};


void DATA_DIAGNOSE(VspdCToMySQL *vspdctomysql, string date_start);
Flow_Diagram Data_Anomaly_predict(VspdCToMySQL *vspdctomysql, string OWNER_CROSS,string DIRECTION, string LANE_CODE, string Date_Nowday, Flow_Diagram Lane_Flow_Iteration, char *split1, char *split2, char *split3);               //数据异常的迭代预测函数
int Data_Latency(VspdCToMySQL *vspdctomysql,string vehicle_pass_table_name,string OWNER_CROSS, string KKBH, string date_start, string date_end,char *split2);   //数据延迟的函数
int Data_Missing(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int lane_flow, string END_TIME_downrange, string END_TIME,  char *split2);
void Data_Anomaly_HMM_judge(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int lane_flow, string END_TIME_downrange, string END_TIME,  char *split2);  //数据异常的写入函数
void Data_Anomaly_SA_judge(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int lane_flow, string END_TIME_downrange, string END_TIME,  char *split2);  //数据异常的写入函数

void Expectation_Maximization(EM_Output *Flow_EM_output,Flow_Diagram Flow_Predict);      //EM算法
void MLM(EM_Output *Flow_EM_output,Flow_Diagram Flow_Predict);                                   //MLM最大似然估计法
float getGaussRatio(float u , float sigma , int x);                       //高斯分布的概率值计算
string Get_Date();                                                        //获取当前系统日期
