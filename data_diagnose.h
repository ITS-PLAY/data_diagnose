#include <string>
#include <cstring>
#include <iostream>
#include <vector>          //����vectorԪ��
using namespace std;             //���string���Ͳ��ܱ�ʶ�������


#define Time_Interval_LATENCY 5       //�ӳ��жϵ�ʱ����
#define Time_Limit_Value 2            //�ӳٵ�ʱ����ֵ
#define data_latency_ratio_Limit 0.05  //�����ӳٵı�����ֵ

#define Time_Interval_MISS 15        //����ȱʧ�жϵ�ʱ����
#define Time_Interval_ANOMALY 15     //�����쳣�жϵ�ʱ����
#define Predictor_Data_Num 1         //�����쳣��Ԥ��δ������ʱ�������ĸ���
#define Gaussian_Num 3               //��ϸ�˹�ֲ��ķֲ�����
#define Confidence_Interval 3        //���������б�׼��ı���
#define PI acos(-1.0)        


struct Flow_Diagram{            //�洢����ֵ�Ͷ�Ӧ��Ƶ��
	vector<int> FLOW_VALUE;
	vector<float> FLOW_FREQUENCY;
};


struct EM_Output{
	float EM_ave;
	float EM_sd;
};


void DATA_DIAGNOSE(VspdCToMySQL *vspdctomysql, string date_start);
Flow_Diagram Data_Anomaly_predict(VspdCToMySQL *vspdctomysql, string OWNER_CROSS,string DIRECTION, string LANE_CODE, string Date_Nowday, Flow_Diagram Lane_Flow_Iteration, char *split1, char *split2, char *split3);               //�����쳣�ĵ���Ԥ�⺯��
int Data_Latency(VspdCToMySQL *vspdctomysql,string vehicle_pass_table_name,string OWNER_CROSS, string KKBH, string date_start, string date_end,char *split2);   //�����ӳٵĺ���
int Data_Missing(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int lane_flow, string END_TIME_downrange, string END_TIME,  char *split2);
void Data_Anomaly_HMM_judge(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int lane_flow, string END_TIME_downrange, string END_TIME,  char *split2);  //�����쳣��д�뺯��
void Data_Anomaly_SA_judge(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int lane_flow, string END_TIME_downrange, string END_TIME,  char *split2);  //�����쳣��д�뺯��

void Expectation_Maximization(EM_Output *Flow_EM_output,Flow_Diagram Flow_Predict);      //EM�㷨
void MLM(EM_Output *Flow_EM_output,Flow_Diagram Flow_Predict);                                   //MLM�����Ȼ���Ʒ�
float getGaussRatio(float u , float sigma , int x);                       //��˹�ֲ��ĸ���ֵ����
string Get_Date();                                                        //��ȡ��ǰϵͳ����
