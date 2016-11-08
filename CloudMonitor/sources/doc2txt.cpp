#include "doc2txt.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <set>
#include <algorithm>

using namespace std;


bool Storage::stream(const char *name, const uchar **stream, uint *size) const {
	for (uint i = 0; i < stream_num; ++i) {
		bool match = true;
		for (uint j = 0; name[j] != '\0'; ++j) {
			if (name[j] != stream_name[i][j * 2]) {
				match = false;
				break;
			}
		}
		if (match) {
			*stream = stream_table[i];
			*size = stream_sizes[i];
			return true;
		}
	}
	return false;
}

bool doc_format(const Storage &st) {
	const uchar *wds, *ts;
	uint wds_size, ts_size;

	// 取得WordDocument Stream (wds)
	if (!st.stream("WordDocument", &wds, &wds_size)) {
		return false;
	}

	// 取得Table Stream (ts)
	ushort flag1 = *(ushort *)((char *)wds + 0xA);
	bool is1Table = (flag1 & 0x0200) == 0x0200;
	if (!st.stream(is1Table ? "1Table" : "0Table", &ts, &ts_size)) {
		return false;
	}

	uint fcPlcfBtePapx = *(uint *)(wds + 258);
	uint lcbPlcfBtePapx = *(uint *)(wds + 258 + 4);
	uint n = (lcbPlcfBtePapx - 4) / 8;

	set< pair<uint, uint> > ranges;
	for (uint i = 0; i < n; ++i) {
		/* 读取PlcBtePapx中的信息（其实PlcBtePapx在Table Stream中） */
		uint PnBtePapx = *(uint *)(ts + fcPlcfBtePapx + 4 * (n + 1 + i));

		const uchar *PapxFkp = wds + PnBtePapx * 512;
		uchar cpara = *(const uchar *)(PapxFkp + 511);

		for (uint i = 0; i < cpara; ++i) {
			// rgfc确定文本的位置
			uint rgfc_start = *(uint *)(PapxFkp + 4 * i);
			uint rgfc_end = *(uint *)(PapxFkp + 4 * i + 4);

			// bOffset找到PapxInFkp在PapxFkp中的位置
			uchar bOffset = *(uchar *)(PapxFkp + 4 * (cpara + 1) + 13 * i);

			// 找到PapxInFkp其中的cb, grpprlInPapx
			uchar cb = *(uchar *)(PapxFkp + bOffset * 2);

			// 得到GrpPrlAndIstd
			uint lcbGrpPrlAndIstd = 2 * cb - 1;
			const uchar *GrpPrlAndIstd = PapxFkp + bOffset * 2 + 1;
			if (cb == 0) {
				lcbGrpPrlAndIstd = *GrpPrlAndIstd * 2;
				GrpPrlAndIstd += 1;
			}

			// 根据GrpPrlAndIstd得到段落格式（Direct Paragraph Formatting）
			const uchar *grpprl = GrpPrlAndIstd + 2;
			uint lcb = lcbGrpPrlAndIstd - 1;
			static const int operand_size[8] = { 1, 1, 2, 4, 2, 2, -1, 3, };
			for (uint i = 0; i < lcb;) {
				ushort sprm = *(ushort *)(grpprl + i);
				uint spra = sprm / 8192;
				uint operand;
				i += 2;
				if (spra == 6) {
					i += 1 + *(uchar *)(grpprl + i);
				}
				else {
					if (operand_size[spra] == 1) {
						operand = *(uchar *)(grpprl + i);
					}
					else if (operand_size[spra] == 2) {
						operand = *(ushort *)(grpprl + i);
					}
					else if (operand_size[spra] == 3) {
						operand = *(uint *)(grpprl + i) & 0x00FFFFFF;
					}
					else if (operand_size[spra] == 4) {
						operand = *(uint *)(grpprl + i);
					}
					else {
						operand = -1;
					}
					i += operand_size[spra];
				}
				if ((sprm == 0x2403 || sprm == 0x2416) && operand == 1) { // 居中
					ranges.insert(make_pair(rgfc_start, rgfc_end));
				}
			}
		}
	}

	/* 合并各个区间 */
	set< pair<uint, uint> >::iterator iter = ranges.begin(), jter, kter;
	while (iter != ranges.end()) {
		jter = iter;
		++jter;
		while (jter != ranges.end() && iter->second == jter->first) {
			pair<uint, uint> temp = *iter;
			temp.second = jter->second;
			ranges.erase(iter);
			ranges.insert(temp);
			iter = ranges.find(temp);
			kter = jter;
			++jter;
			ranges.erase(kter);
		}
		iter = jter;
	}

	return true;
}

// 提取一个OfficeArtBStoreContainerFileBlock中的图片
static bool storeBlipBlock(const uchar *block, const char *img_name, uint size) {
	// uchar recVer = *(uchar *)(block) & 0xF;
	ushort recInstance = *(ushort *)(block) >> 4;
	ushort recType = *(ushort *)(block + 2);
	// ushort recLen = *(uint *)(block + 4);

	char str[128];
	const uchar *start = block;
	if (recType == 0xF01A) { // EMF
		sprintf(str, "%s.emf", img_name);
		start += 8 + 16 + (recInstance == 0x3D5 ? 16 : 0) + 34;
	}
	else if (recType == 0xF01B) { // WMF
		sprintf(str, "%s.wmf", img_name);
		start += 8 + 16 + (recInstance == 0x217 ? 16 : 0) + 34;
	}
	else if (recType == 0xF01C) { // PICT
		sprintf(str, "%s.pict", img_name);
		start += 8 + 16 + (recInstance == 0x543 ? 16 : 0) + 34;
	}
	else if (recType == 0xF01D || recType == 0xF02A) { // JPEG
		sprintf(str, "%s.jpeg", img_name);
		start += (recInstance == 0x46B || recInstance == 0x6E3 ? 41 : 25);
	}
	else if (recType == 0xF01E) { // PNG
		sprintf(str, "%s.png", img_name);
		start += 8 + 16 + (recInstance == 0x6E1 ? 16 : 0) + 1;
	}
	else if (recType == 0xF01F) { // DIB
		sprintf(str, "%s.dib", img_name);
		start += 8 + 16 + (recInstance == 0x7A9 ? 16 : 0) + 1;
	}
	else if (recType == 0xF029) { // TIFF
		sprintf(str, "%s.tiff", img_name);
		start += 8 + 16 + (recInstance == 0x6E5 ? 16 : 0) + 1;
	}

	FILE *file = fopen(str, "wb");
	for (uint i = 0; i < size; ++i) {
		fputc(start[i], file);
	}
	fclose(file);
	return true;
}

static bool retrieve_image(const Storage &st, const char *file_path) {
	const uchar *wds, *ts;
	uint wds_size, ts_size;

	// 取得WordDocument Stream (wds)
	if (!st.stream("WordDocument", &wds, &wds_size)) {
		return false;
	}

	// 取得Table Stream (ts)
	ushort flag1 = *(ushort *)((char *)wds + 0xA);
	bool is1Table = (flag1 & 0x0200) == 0x0200;
	if (!st.stream(is1Table ? "1Table" : "0Table", &ts, &ts_size)) {
		return false;
	}

	uint fcDggInfo = *(const uint *)(wds + 154 + (194 - 94) * 4);
	uint lcbDggInfo = *(const uint *)(wds + 154 + (194 - 94) * 4 + 4);
	if (lcbDggInfo == 0 || fcDggInfo >= ts_size) {
		return false;
	}

	const uchar *OfficeArtContent = ts + fcDggInfo;
	const uchar *blipStore = OfficeArtContent + 16 + *(const uint *)(OfficeArtContent + 12);
	uint lcbBlipStore = *(uint *)(blipStore + 4);
	// printf("lcbBlipStore = %u\n", lcbBlipStore);

	const uchar *rgfb = blipStore + 8;
	char *img_name = (char *)malloc(strlen(file_path) + 16);
	while (rgfb < blipStore + lcbBlipStore) {
		static int counter = 0;
		sprintf(img_name, "%s/%05d", file_path, ++counter);
		ushort recType = *(const ushort *)(rgfb + 2);
		uint recSize = *(const uint *)(rgfb + 4);
		/*
		printf("recType = %04X\n", recType);
		printf("recSize = %u\n", recSize);
		*/
		if (recType == 0xF007) {
			uint size = *(const uint *)(rgfb + 28);
			uint foDelay = *(const uint *)(rgfb + 36);
			// printf("size = %u, foDelay = %u\n", size, foDelay);
			if (foDelay < wds_size && foDelay + size <= wds_size) {
				storeBlipBlock(wds + foDelay, img_name, size);
			}
		}
		else if (0xF018 <= recType && recType <= 0xF117) {
			storeBlipBlock(rgfb, img_name, recSize);
		}
		else {
			break;
		}
		rgfb += 8 + *(const uint *)(rgfb + 4);
	}
	free(img_name);
	return true;
}

static bool inline_image(const Storage &st, const char *file_path) {
	const uchar *wds, *ts, *ds;
	uint wds_size, ts_size, ds_size;

	// 取得Data Stream (ds)
	if (!st.stream("Data", &ds, &ds_size)) {
		return false;
	}

	// 取得WordDocument Stream (wds)
	if (!st.stream("WordDocument", &wds, &wds_size)) {
		return false;
	}

	// 取得Table Stream (ts)
	ushort flag1 = *(ushort *)((char *)wds + 0xA);
	bool is1Table = (flag1 & 0x0200) == 0x0200;
	if (!st.stream(is1Table ? "1Table" : "0Table", &ts, &ts_size)) {
		return false;
	}


	uint fcPlcfBteChpx = *(uint *)(wds + 250);
	uint lcbPlcfBteChpx = *(uint *)(wds + 250 + 4);
	int n = (lcbPlcfBteChpx - 4) / 8;

	for (int i = 0; i < n; ++i) {
		uint PnBteChpx = *(uint *)(ts + fcPlcfBteChpx + 4 * (n + 1 + i));
		const uchar *ChpxFkp = wds + PnBteChpx * 512;
		uchar crun = *(const uchar *)(ChpxFkp + 511);

		for (uint i = 0; i < crun; ++i) {
			// rgfc确定文本的位置
			uint rgfc_start = *(const uint *)(ChpxFkp + 4 * i);
			ushort ch = *(const ushort *)(wds + rgfc_start);
			if (ch != 0x0001) {
				continue;
			}

			// bOffset找到ChpxInFkp在ChpxFkp中的位置
			uchar bOffset = *(const uchar *)(ChpxFkp + 4 * (crun + 1) + 1 * i);
			if (bOffset == 0) {
				continue;
			}

			// 找到ChpxInFkp其中的lcb, grpprlInChpx
			uchar lcb = *(const uchar *)(ChpxFkp + bOffset * 2);
			const uchar *grpprl = ChpxFkp + 2 * bOffset + 1;
			static const int operand_size[8] = { 1, 1, 2, 4, 2, 2, -1, 3, };
			for (uint i = 0; i < lcb;) {
				ushort sprm = *(ushort *)(grpprl + i);
				uint spra = sprm / 8192;
				uint operand;
				i += 2;
				if (spra == 6) {
					i += 1 + *(const uchar *)(grpprl + i);
				}
				else {
					if (operand_size[spra] == 1) {
						operand = *(const uchar *)(grpprl + i);
					}
					else if (operand_size[spra] == 2) {
						operand = *(const ushort *)(grpprl + i);
					}
					else if (operand_size[spra] == 3) {
						operand = *(const uint *)(grpprl + i) & 0x00FFFFFF;
					}
					else if (operand_size[spra] == 4) {
						operand = *(const uint *)(grpprl + i);
					}
					else {
						operand = -1;
					}
					i += operand_size[spra];
				}

				if (sprm != 0x6A03) {
					continue;
				}

				const uchar *PICF = ds + operand;
				uint lcbPICF = *(const uint *)PICF;

				ushort picf_mfpf_mm = *(const ushort *)(PICF + 6);
				const uchar *picture = PICF + 68 + (picf_mfpf_mm == 0x0066 ? 1 + *(PICF + 68) : 0);
				const uchar *rgfb = picture + 8 + *(const uint *)(picture + 4);

				char *img_name = (char *)malloc(strlen(file_path) + 16);
				while (PICF <= rgfb && rgfb < PICF + lcbPICF) {
					static int counter = 0;
					sprintf(img_name, "%s/i%05d", file_path, ++counter);
					uint recSize = *(const uint *)(rgfb + 4);
					uchar cbName = *(rgfb + 41);
					storeBlipBlock(rgfb + 44, img_name, recSize - cbName - 36);
					rgfb += 8 + recSize;
				}
				free(img_name);
			}
		}
	}
	return true;
}

bool doc_image(const Storage &storage, const char *image_dir_path) {
	retrieve_image(storage, image_dir_path);
	inline_image(storage, image_dir_path);
	return true;
}


bool parse_summary_information(const Storage &st) {
	// 取得\005SummaryInformation Stream
	char sis_name[64];
	sis_name[0] = 5, sis_name[1] = 0;
	sprintf(sis_name + 2, "SummaryInformation");
	const uchar *sis;
	uint sis_size;
	if (!st.stream(sis_name, &sis, &sis_size)) {
		return false;
	}


	FILE *out_file = fopen("./property.txt", "wb");

	const uchar *ps = sis + 48;
	uint pre_size = 0;
	uint num_property_set = *(uint *)(sis + 24);
	for (uint set_id = 0; set_id < num_property_set; ++set_id) {
		ps += pre_size;
		pre_size = *(uint *)(ps);
		uint num_properties = *(uint *)(ps + 4);
		printf("num_properties = %u\n", num_properties);
		for (uint i = 0; i < num_properties; ++i) {
			uint pid = *(uint *)(ps + 8 + 8 * i);
			uint offset = *(uint *)(ps + 8 + 8 * i + 4);
			ushort type = *(ushort *)(ps + offset);
			ushort padding = *(ushort *)(ps + offset + 2);
			printf("pid = %u, offset = %u, type = 0x%04X, padding = %d\n",
				pid, offset, type, padding);
			const uchar *value = ps + offset + 4;
			if (type == 0x001E) {
				uint size = *(uint *)(value);
				// printf("size = %u\n", size);
				printf("-> %s\n", (char *)(value + 4));
				for (uint i = 0; i < size; ++i) {
					// fputc(value[4 + i], out_file);
					// printf("value[0x%02X] = 0x%02X\n", 4 + i, value[4 + i]);
					// fputc(value[4 + i], file);
				}
			}
		}
	}
	fclose(out_file);
	return true;
}

bool doc_property(const Storage &st) {
	return parse_summary_information(st);
}

static const uchar *get_piece_table(const Storage &storage) {
	const uchar *wds, *ts;
	uint wds_size, ts_size;
	if (!storage.stream("WordDocument", &wds, &wds_size)) {
		return NULL;
	}
	ushort flag1 = *(const ushort *)(wds + 0x0A);
	bool is1Table = (flag1 & 0x0200) == 0x0200;
	if (!storage.stream(is1Table ? "1Table" : "0Table", &ts, &ts_size)) {
		return NULL;
	}

	uint fcClx = *(const uint *)(wds + 0x01A2);
	uint lcbClx = *(const uint *)(wds + 0x01A6);
	const uchar *clx = ts + fcClx;
	for (uint pos = 0; clx + pos < ts + ts_size && pos < lcbClx;) {
		uchar type = clx[pos];
		if (type == 2) {
			return clx + pos;
		}
		else if (type == 1) {
			pos += 3 + *(const ushort *)(clx + pos + 1);
		}
		else {
			return NULL;
		}
	}
	return NULL;
}

/*
* 提取文本块，不处理其中的非法字符，返回一个指向文本段的指针
*/
uchar *doc_text(const Storage &storage) {
	const uchar *wds;
	uint wds_size;
	if (!storage.stream("WordDocument", &wds, &wds_size)) {
		return NULL;
	}

	const uchar *pt = get_piece_table(storage);
	if (pt == NULL) {
		return NULL;
	}

	uint buf_size = 0; // 文本块的大小

	uint lcb = *(const uint *)(pt + 1);
	uint n = (lcb - 4) / 12;
	const uchar *pcd = pt + 5;
	for (uint i = 0; i < n; ++i) {
		uint start = *(const uint *)(pcd + i * 4);
		uint end = *(const uint *)(pcd + i * 4 + 4);
		if (start >= end) return NULL;
		uint fcCompressed = *(const uint *)(pcd + (n + 1) * 4 + i * 8 + 2);
		bool fCompressed = (fcCompressed >> 30) & 1;
		if (fCompressed) {
			uint cb = end - start;
			buf_size += cb * 2;
		}
		else {
			uint cb = (end - start) * 2;
			buf_size += cb;
		}
	}

	uchar *buf = (uchar *)malloc(buf_size + 4);
	uint ptr = 0;
	if (buf == NULL) return NULL;

	for (uint i = 0; i < n; ++i) {
		uint start = *(const uint *)(pcd + i * 4);
		uint end = *(const uint *)(pcd + i * 4 + 4);
		uint fcCompressed = *(const uint *)(pcd + (n + 1) * 4 + i * 8 + 2);
		bool fCompressed = (fcCompressed >> 30) & 1;
		if (fCompressed) {
			uint fc = (fcCompressed & ~(1U << 30)) / 2;
			uint cb = end - start;
			for (uint i = 0; i < cb; ++i) {
				if (fc + i < wds_size) {
					buf[ptr++] = wds[fc + i];
					buf[ptr++] = 0;
				}
			}
		}
		else {
			uint fc = fcCompressed;
			uint cb = (end - start) * 2;
			for (uint i = 0; i < cb; ++i) {
				buf[ptr++] = wds[fc + i];
			}
		}
	}
	return buf;
}

/*
* 提取文本块，到指定文件中
*/
bool doc_text_file(const Storage &storage, FILE *text_file) {
	const uchar *wds;
	uint wds_size;
	if (!storage.stream("WordDocument", &wds, &wds_size)) {
		return false;
	}

	const uchar *pt = get_piece_table(storage);
	if (pt == NULL) {
		return false;
	}

	fputc(0xFF, text_file);
	fputc(0xFE, text_file);
	uint lcb = *(const uint *)(pt + 1);
	uint n = (lcb - 4) / 12;
	const uchar *pcd = pt + 5;
	for (uint i = 0; i < n; ++i) {
		uint start = *(const uint *)(pcd + i * 4);
		uint end = *(const uint *)(pcd + i * 4 + 4);
		if (start >= end) return false;
		uint fcCompressed = *(const uint *)(pcd + (n + 1) * 4 + i * 8 + 2);
		bool fCompressed = (fcCompressed >> 30) & 1;
		if (fCompressed) {
			uint fc = (fcCompressed & ~(1U << 30)) / 2;
			uint cb = end - start;
			for (uint i = 0; i < cb; ++i) {
				if (fc + i < wds_size) {
					fputc(wds[fc + i], text_file);
					fputc(0, text_file);
				}
			}
		}
		else {
			uint fc = fcCompressed;
			uint cb = (end - start) * 2;
			for (uint i = 0; i < cb; i += 2) {
				if (fc + i + 1 < wds_size) {
					uchar low = wds[fc + i], high = wds[fc + i + 1];
					ushort ch = ((ushort)high << 8) + low;
					if (ch >= 0x000A) {
						fputc(wds[fc + i], text_file);
						fputc(wds[fc + i + 1], text_file);
					}
				}
			}
		}
	}
	return true;
}


bool parse_doc(const char *doc_file_path, const char *text_file_path, const char *image_dir_path) {
	FILE *doc_file = fopen(doc_file_path, "rb");
	if (doc_file == NULL) {
		return false;
	}

	/* 初始化流 */
	Storage storage;
	if (!storage.init(doc_file)) {
		fclose(doc_file);
		return false;
	}
	fclose(doc_file);

	/* 提取文本 */
	FILE *text_file = fopen(text_file_path, "wb");
	if (text_file == NULL) {
		return false;
	}
	doc_text_file(storage, text_file);
	fclose(text_file);

	if (NULL != image_dir_path) {
		/* 提取图片 */
		doc_image(storage, image_dir_path);
	}

	/* 分析格式 */
	//doc_format(storage);

	/* 标题、作者等信息 */
	//doc_property(storage);
	return true;
}


Storage::Storage() {
	stream_num = 0U;
	stream_name = NULL;
	stream_table = NULL;
	stream_sizes = NULL;
}

Storage::~Storage() {
	for (uint i = 0; i < stream_num; ++i) {
		if (stream_name != NULL && stream_name[i] != NULL) {
			free(stream_name[i]);
			stream_name[i] = NULL;
		}
		if (stream_table != NULL && stream_table[i] != NULL) {
			free(stream_table[i]);
			stream_table[i] = NULL;
		}
	}
	stream_num = 0;
	if (stream_name != NULL) {
		free(stream_name);
		stream_name = NULL;
	}
	if (stream_table != NULL) {
		free(stream_table);
		stream_table = NULL;
	}
	if (stream_sizes != NULL) {
		free(stream_sizes);
		stream_sizes = NULL;
	}
}

static bool read_buf(FILE *doc_file, uchar **buf, uint *buf_size) {
	fseek(doc_file, 0, SEEK_END);
	*buf_size = ftell(doc_file);
	if (*buf_size < 512) {
		return false;
	}
	*buf_size = ((*buf_size - 512) + 511) / 512 * 512 + 512;
	if ((*buf = (uchar *)malloc(*buf_size)) == NULL) {
		return false;
	}
	fseek(doc_file, 0, SEEK_SET);
	int ch, i = 0;
	while ((ch = fgetc(doc_file)) != EOF) {
		*(*buf + i++) = (uchar)ch;
	}
	return true;
}

static bool read_msat(uchar *buf, uint buf_size, uchar **msat, uint *msat_size) {
	set<uint> vis;
	uint num_sec = 0, id = *(uint *)(buf + 0x44);
	for (; id < buf_size / 512 - 1; id = *(uint *)(buf + 512 + 512 * id + 508)) {
		if (vis.count(id)) {
			return false;
		}
		vis.insert(id);
		num_sec += 1;
	}
	*msat_size = 109 * 4 + num_sec * 512;
	if ((*msat = (uchar *)malloc(*msat_size)) == NULL) {
		return false;
	}
	memcpy(*msat, buf + 0x4C, 109 * 4);
	num_sec = 0;
	id = *(int *)(buf + 0x44);
	for (; id < buf_size / 512 - 1; id = *(int *)(buf + 512 + 512 * id + 508)) {
		memcpy(*msat + 436 + num_sec++ * 508, buf + 512 + 512 * id, 508);
	}
	return true;
}

static bool read_sat(uchar *buf, uint buf_size, uchar *msat, uint msat_size,
	uchar **sat, uint *sat_size) {
	*sat_size = 0;
	for (uint i = 0; i < msat_size / 4; ++i) {
		uint id = *(uint *)(msat + i * 4);
		if (id > buf_size / 512 - 1) break;
		*sat_size += 1;
	}
	*sat_size *= 512;
	if ((*sat = (uchar *)malloc(*sat_size)) == NULL) {
		return false;
	}
	for (uint i = 0; i < msat_size / 4; ++i) {
		uint id = *(uint *)(msat + i * 4);
		if (id > buf_size / 512 - 1) break;
		memcpy(*sat + i * 512, buf + 512 + id * 512, 512);
	}
	return true;
}

static bool read_stream(uint start, uint sec_sz, uchar *sat, uint sat_size,
	uchar *buf, uint buf_size, uchar **stream, uint *sz) {
	uint sec_num = 0;
	set<uint> vis;
	for (uint id = start; id < buf_size / sec_sz; id = *(uint *)(sat + id * 4)) {
		if (vis.count(id)) return false;
		vis.insert(id);
		++sec_num;
	}
	*sz = sec_num * sec_sz;
	*stream = (uchar *)malloc(*sz);
	sec_num = 0;
	for (uint id = start; id < buf_size / sec_sz; id = *(uint *)(sat + id * 4)) {
		memcpy(*stream + sec_num++ * sec_sz, buf + id * sec_sz, sec_sz);
	}
	return true;
}

bool Storage::init(FILE *doc_file) {
	if (doc_file == NULL) {
		return false;
	}

	uchar *buf, *mbuf, *msat, *sat, *ssat, *dir_stream;
	uint buf_size, mbuf_size, msat_size, sat_size, ssat_size, dir_stream_size;

	if (!read_buf(doc_file, &buf, &buf_size)) return false;
	if (!read_msat(buf, buf_size, &msat, &msat_size)) return false;
	if (!read_sat(buf, buf_size, msat, msat_size, &sat, &sat_size)) return false;
	free(msat);

	/* Directory Stream */
	if (!read_stream(*(uint *)(buf + 0x30), 512, sat, sat_size,
		buf + 512, buf_size - 512, &dir_stream, &dir_stream_size)) {
		return false;
	}
	if (dir_stream_size < 128) return false;

	/* short-sector Stream's mbuf and ssat */
	if (!read_stream(*(uint *)(dir_stream + 0x74), 512, sat, sat_size,
		buf + 512, buf_size - 512, &mbuf, &mbuf_size)) {
		return false;
	}
	if (!read_stream(*(uint *)(buf + 0x3C), 512, sat, sat_size,
		buf + 512, buf_size - 512, &ssat, &ssat_size)) {
		return false;
	}

	/* all streams */
	uint _ulMiniSectorCutoff = *(uint *)(buf + 0x38);

	stream_num = dir_stream_size / 128 - 1;
	stream_name = (uchar **)malloc(stream_num * sizeof(uchar *));
	stream_table = (uchar **)malloc(stream_num * sizeof(uchar *));
	stream_sizes = (uint *)malloc(stream_num * sizeof(uint));

	memset(stream_name, 0, stream_num * sizeof(uchar *));
	memset(stream_table, 0, stream_num * sizeof(uchar *));
	memset(stream_sizes, 0, stream_num * sizeof(uchar *));

	for (uint i = 0; i < stream_num; ++i) {
		uchar *dir_entry = dir_stream + i * 128;
		stream_name[i] = (uchar *)malloc(64);
		memcpy(stream_name[i], dir_entry, 64);
		if (i == 0) continue;
		uint id = *(uint *)(dir_entry + 0x74);
		stream_sizes[i] = *(uint *)(dir_entry + 0x78);
		/*
		for (uint j = 0; j < 32; ++j) {
		printf("%c", stream_name[i][j * 2]);
		}
		printf("-> sz=%u, id=%u\n", stream_sizes[i], id);
		*/
		if (stream_sizes[i] > _ulMiniSectorCutoff) {
			if (!read_stream(id, 512, sat, sat_size,
				buf + 512, buf_size - 512, &stream_table[i], &stream_sizes[i])) {
				return false;
			}
		}
		else {
			if (!read_stream(id, 64, ssat, ssat_size,
				mbuf, mbuf_size, &stream_table[i], &stream_sizes[i])) {
				return false;
			}
		}
	}

	free(buf);
	free(mbuf);
	free(sat);
	free(ssat);
	free(dir_stream);
	return true;
}

// Convert Unicode big endian to Unicode little endian
// http://www.cnblogs.com/jojodru/archive/2012/07/03/2574616.html
unsigned Ucs2BeToUcs2Le(unsigned short *ucs2bige, unsigned int size)
{
	printf("%s %d\n", __FUNCTION__, __LINE__);

	if (!ucs2bige) {
		return 0;
	}

	unsigned int length = size;
	unsigned short *tmp = ucs2bige;

	while (*tmp && length) {

		length--;
		unsigned char val_high = *tmp >> 8;
		unsigned char val_low = (unsigned char)*tmp;

		*tmp = val_low << 8 | val_high;

		tmp++;
	}

	return size - length;
}

// Convert Ucs-2 to Utf-8
unsigned int Ucs2ToUtf8(unsigned short *ucs2, unsigned int ucs2_size,
	unsigned char *utf8, unsigned int utf8_size)
{
	unsigned int length = 0;

	if (!ucs2) {
		return 0;
	}

	unsigned short *inbuf = ucs2;
	unsigned char *outbuf = utf8;

	if (*inbuf == 0xFFFE) {
		Ucs2BeToUcs2Le(inbuf, ucs2_size);
	}

	if (!utf8) {
		unsigned int insize = ucs2_size;

		while (*inbuf && insize) {
			insize--;

			/*            if (*inbuf == 0xFEFF) {
			inbuf++;
			continue;
			}*/

			if (0x0080 > *inbuf) {
				length++;
			}
			else if (0x0800 > *inbuf) {
				length += 2;
			}
			else {
				length += 3;
			}

			inbuf++;
		}
		return length;

	}
	else {
		unsigned int insize = ucs2_size;

		while (*inbuf && insize && length < utf8_size) {
			insize--;

			if (*inbuf == 0xFFFE) {
				inbuf++;
				continue;
			}

			if (0x0080 > *inbuf) {
				/* 1 byte UTF-8 Character.*/
				*outbuf++ = (unsigned char)(*inbuf);
				length++;
			}
			else if (0x0800 > *inbuf) {
				/*2 bytes UTF-8 Character.*/
				*outbuf++ = 0xc0 | ((unsigned char)(*inbuf >> 6));
				*outbuf++ = 0x80 | ((unsigned char)(*inbuf & 0x3F));
				length += 2;

			}
			else {
				/* 3 bytes UTF-8 Character .*/
				*outbuf++ = 0xE0 | ((unsigned char)(*inbuf >> 12));
				*outbuf++ = 0x80 | ((unsigned char)((*inbuf >> 6) & 0x3F));
				*outbuf++ = 0x80 | ((unsigned char)(*inbuf & 0x3F));
				length += 3;
			}

			inbuf++;
		}

		return length;
	}
}

// Convert Utf-8 to Ucs-2
unsigned int Utf8ToUcs2(unsigned char *utf8, unsigned int utf8_size,
	unsigned short *ucs2, unsigned int ucs2_size)
{
	unsigned int length = 0;
	unsigned int insize = utf8_size;
	unsigned char *inbuf = utf8;

	if (!utf8)
		return 0;

	if (!ucs2) {
		while (*inbuf && insize) {
			unsigned char c = *inbuf;
			if ((c & 0x80) == 0) {
				length += 1;
				insize -= 1;
				inbuf++;
			}
			else if ((c & 0xE0) == 0xC0) {
				length += 1;
				insize -= 2;
				inbuf += 2;
			}
			else if ((c & 0xF0) == 0xE0) {
				length += 1;
				insize -= 3;
				inbuf += 3;
			}
		}
		return length;

	}
	else {
		unsigned short *outbuf = ucs2;
		unsigned int outsize = ucs2_size;

		//while(*inbuf && insize && length < outsize) {
		while (insize && length < outsize) {
			unsigned char c = *inbuf;
			if ((c & 0x80) == 0) {
				*outbuf++ = c;
				inbuf++;
				length++;
				insize--;
			}
			else if ((c & 0xE0) == 0xC0) {
				unsigned short val;

				val = (c & 0x3F) << 6;
				inbuf++;
				c = *inbuf;
				val |= (c & 0x3F);
				inbuf++;

				length++;
				insize -= 2;

				*outbuf++ = val;
			}
			else if ((c & 0xF0) == 0xE0) {
				unsigned short val;

				val = (c & 0x1F) << 12;
				inbuf++;
				c = *inbuf;
				val |= (c & 0x3F) << 6;
				inbuf++;
				c = *inbuf;
				val |= (c & 0x3F);
				inbuf++;

				insize -= 3;
				length++;

				*outbuf++ = val;
			}
		}
		return length;
	}
	return 0;
}


static int DumpToFile(char *FileName, char *buf, size_t FileSize)
{
	FILE     *fp;

	if ((fp = fopen(FileName, "wb")) == NULL)
	{
		return -1;
	}
	fwrite(buf, 1, FileSize, fp);

	fclose(fp);

	return 0;
}

static int DumpFromFile(char *FileName, char *buf, size_t FileSize)
{
	FILE     *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
	{
		return -1;
	}
	fread(buf, 1, FileSize, fp);

	fclose(fp);

	return 0;
}

static int GetFileSize(char *FileName, size_t *FileSize)
{
	FILE *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
		return -1;

	//printf("%s open OK\n", FileName);
	fseek(fp, 0, SEEK_END);
	*FileSize = ftell(fp);

	fclose(fp);
	fp = NULL;

	return 0;
}

int DecodeUcs2Utf8(char *FileName)
{
	size_t FileSize = 0;
	int    ret;
	char *buf1 = NULL;
	char *buf2 = NULL;

	GetFileSize(FileName, &FileSize);

	// .doc  文件,使用 ucs2 编码，每个汉字占用 2 bytes
	// .docx 文件,使用 utf8 编码，每个汉字占用 3 bytes
	// 申请总空间 = ucs2_FileSize + 1.5 * ucs_FileSize = 2.5 *ucs_FileSize
	// 为保证鲁棒性,申请三倍空间
	if ((buf1 = (char *)malloc(FileSize * 3)) == NULL)
	{
		perror("malloc");
		exit(1);
	}
	buf2 = buf1 + FileSize;

	DumpFromFile(FileName, buf1, FileSize);
	size_t fs2 = Ucs2ToUtf8((unsigned short *)buf1, FileSize / 2, (unsigned char *)buf2, FileSize * 2);
	ret = DumpToFile(FileName, buf2, fs2);
	free(buf1);

	return ret;
}

int ParseDoc(char *docFileName, char *txtFileName)
{

	if (!parse_doc(docFileName, txtFileName, NULL))
	{
		perror(docFileName);
		return -1;
	}
	DecodeUcs2Utf8(txtFileName);

	//printf("parsing %s to %s generated Ok!\n", docFileName, txtFileName);
	return 0;
}
