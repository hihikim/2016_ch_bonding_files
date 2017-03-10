import xlsxwriter
import os

ap_dir_path = './output/ap/'
sta_dir_path = './output/sta/'
log_dir_path = './input/log/'


algorithms = ['opt','proposed','waterfall']

class Parser:
    def __init__(self):
        self.workbook = xlsxwriter.Workbook("Output_Arrange.xlsx")
        self.output_files = []

    def Parsing(self):
        for root, dirs, files in os.walk(ap_dir_path + algorithms[0] +'/') :
            for file_name in files:
                self.output_files.append(file_name)

        self.output_files.sort()

        for test_number in self.output_files:
            self.test_number = test_number
            self.GetData()
            self.PrintData()


        self.workbook.close()

    def GetData(self):
        self.result = {}
        for algo in algorithms:
            path = ap_dir_path + algo + '/' + self.test_number
            log_path = log_dir_path + algo + '/' + self.test_number
            ap_path = './input/ap/' + algo + '/' + self.test_number
            self.input_aps = self.GetAps(ap_path)
            self.result[algo] = self.ParseOutput(path,log_path)

    def ParseOutput(self,path,log_path):
        output_file = open(path,'r')
        log_file = open(log_path,'r')
        result = {}
        log_val = []

        line =  log_file.readline()
        split_val = line.split()

        for val in split_val:
            value = float(val)
            log_val.append(value)


        ap_index = -1

        for line in output_file.readlines():
            if not line.find('---') > -1 :
                val_index = line.find(' :')

                if val_index == -1 :
                    continue

                token = line[0:val_index]
                value = line[val_index+3:]
                value = value.replace('\n', '')

                if token == 'Time' :
                    continue

                elif token == 'index' :
                    ap_index = int(value)
                    if ap_index not in result:
                        result[ap_index] = {}


                elif token == 'average throughput':
                    result[ap_index]['avg'] = float(value)

                elif token == 'minimum throughput':
                    result[ap_index]['min'] = float(value)

                elif token == 'maximum throughput':
                    result[ap_index]['max'] = float(value)

        for index in range(len(log_val)):
            ap_index = self.input_aps[index]

            if ap_index not in result:
                result[ap_index] = {'avg':0.0,'min':0.0,'max':0.0,'predict':log_val[index]}

            else :
                result[ap_index]['predict'] = log_val[index]


        return result

    def GetAps(self,ap_path):
        ap_file = open(ap_path,'r')
        result = []
        for line in ap_file.readlines():
            ignore_index = line.find('#')
            if ignore_index > -1:
                line = line[:ignore_index]

            ap_info_line = line.replace(',',' ')
            ap_info = ap_info_line.split()

            if len(ap_info) > 0 :
                result.append(int(ap_info[0]))

        result.sort()
        return result

    def PrintData(self):
        start_row = 4
        start_col = 3
        center_format = self.workbook.add_format({'align':'center', 'valign': 'vcenter'})
        worksheet = self.workbook.add_worksheet(self.test_number)

        ap_index = 0

        worksheet.merge_range(start_row-1, start_col -1, start_row, start_col-1, 'AP', center_format)
        worksheet.write(start_row+1, start_col -1, 'predict', center_format)
        worksheet.write(start_row+2, start_col -1, 'minimum', center_format)
        worksheet.write(start_row+3, start_col -1, 'maximum', center_format)
        worksheet.write(start_row+4, start_col -1, 'average', center_format)


        query = {}
        for algo in algorithms:
            query[algo] = {}
            query[algo]['min'] = '=SUM('
            query[algo]['max'] = '=SUM('
            query[algo]['avg'] = '=SUM('

        for ap in self.result[algorithms[0]].keys():
            worksheet.merge_range(start_row - 1, start_col + (ap_index * 3), start_row - 1, start_col + (ap_index * 3 + 2), ap, center_format)
            for algo in range(len(algorithms)):
                worksheet.write(start_row, start_col + (ap_index * 3 + algo), algorithms[algo], center_format)
                worksheet.write(start_row+1, start_col + (ap_index * 3 + algo), self.result[algorithms[algo]][ap]['predict'], center_format)
                worksheet.write(start_row+2, start_col + (ap_index * 3 + algo), self.result[algorithms[algo]][ap]['min'], center_format)
                worksheet.write(start_row+3, start_col + (ap_index * 3 + algo), self.result[algorithms[algo]][ap]['max'], center_format)
                worksheet.write(start_row+4, start_col + (ap_index * 3 + algo), self.result[algorithms[algo]][ap]['avg'], center_format)

                query[algorithms[algo]]['min'] += (self.get_cell_addr(start_row+2 , start_col + (ap_index * 3 + algo)) + ",")
                query[algorithms[algo]]['max'] += (self.get_cell_addr(start_row+3 , start_col + (ap_index * 3 + algo)) + ",")
                query[algorithms[algo]]['avg'] += (self.get_cell_addr(start_row+4 , start_col + (ap_index * 3 + algo)) + ",")

            ap_index += 1

        algo_num = 0
        opt_list = ['min','max', 'avg']
        for algo in query:
            opt_num = 0
            worksheet.write(start_row + 9 , start_col + algo_num , algo, center_format)

            for opt in opt_list:
                query[algo][opt] = query[algo][opt][:-1] + ')'
                worksheet.write(start_row + 10 + opt_num, start_col + algo_num, query[algo][opt], center_format)
                if algo_num is 0:
                    worksheet.write(start_row + 10 + opt_num, start_col - 1, opt, center_format)


                opt_num += 1

            algo_num += 1

    def get_cell_addr(self,row,col):
        row += 1
        result = ""

        while True :
            remain = col % 26
            col /= 26
            remain += ord("A")

            result = str(chr(remain)) + result
            if col is 0:
                break

        result += str(row)
        return result



Parser = Parser()
Parser.Parsing()

