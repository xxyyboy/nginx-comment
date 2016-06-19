
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * 1.��������һ�ַ�˯������Ҳ����˵���ĳ��������ͼ��ȡ�������������Ѿ����������̻�ã���ô
 * ����ʹ��ǰ���̽���˯��״̬������ʼ�ձ��ֽ����ڿ�ִ��״̬��ÿ���ں˵��ȵ��ý���ִ��ʱ��
 * ���̻��������Ƿ���Ի�ȡ������
 * ��ȡ������ʱ���ý��̻�һֱ�����������봦ִ�У�ֱ�����������ͷ����ҵ�ǰ���̻�ȡ��������Ż�
 * ��������ִ��
 * 2.������Ҫ����Ĺ�����Դ����������ÿ������ʹ������ʱ��ǳ��̣���ʹ�����Ľ��̲�ϣ���Լ�û�� 
 * ��ȡ������ʱ����뵽˯��״̬
 * 3.��������ò����������ܻ�ʹĳһ�������޷�ִ�У���epoll�ϵ����������ǿ���ִ�еģ���ʱ����
 * ʹ������������Ӧ��ʹ�÷������Ļ�����
 */

/*������*/
void
ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin)
{

#if (NGX_HAVE_ATOMIC_OPS)

    ngx_uint_t  i, n;

    //�޷���ȡ��ʱ�����̽�һֱ����δ�����ִ��
    for ( ;; ) {

        /*
         * *mtx->lock == 0 && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid)
         * *mtx->lock == 0����Ŀǰû�н��̳�����������̵�Nginx�����п��ܳ����������:
         * ���ǵ�һ�����(*mtx->lock == 0)ִ�гɹ�,����ִ�еڶ������ǰ������һ�������õ�������
         * ��ʱ�ڶ�������ִ��ʧ�ܣ�������ngx_atomic_cmp_set�����������ж�lockֵ�Ƿ�Ϊ0��ԭ��
         * ֻ��lockֵ��Ϊ0�����ܳɹ���ȡ�������ɹ�����lockֵΪ��ǰ���̵�id
         */
        if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
            return;
        }

        if (ngx_ncpu > 1) {

            for (n = 1; n < spin; n <<= 1) {

                //�����ȴ�һ��ʱ�䣬���ŵȴ���ʱ��Խ�������Ի�ȡ����ʱ����ҲԽ��Խ��
                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();  //ִ���������ǰ����û��"�ó�"������
                }

                if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
                    return;
                }
            }
        }

        /*
         * �������������ʹ��ǰ������ʱ"�ó�"������,���ǵ�ǰ�����Դ��ڿ�ִ��״̬��ֻ���ô�����
         * ���ȵ����������ڿ�ִ��״̬�Ľ��̣������̱��ں��ٴε���ʱ����forѭ�������п��Լ�������
         * ��ȡ��
         */
        
        ngx_sched_yield();
    }

#else

#if (NGX_THREADS)

#error ngx_spinlock() or ngx_atomic_cmp_set() are not defined !

#endif

#endif

}