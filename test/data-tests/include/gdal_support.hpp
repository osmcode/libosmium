#ifndef GDAL_SUPPORT_HPP
#define GDAL_SUPPORT_HPP

#if GDAL_VERSION_MAJOR < 2

typedef OGRSFDriver Driver;
typedef OGRDataSource DataSource;

#define GET_DRIVER_BY_NAME(name) OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(name)
#define CREATE_DATA_SOURCE(driver, filename, options) driver->CreateDataSource(filename, options)
#define DESTROY_DATA_SOURCE(data_source) OGRDataSource::DestroyDataSource(data_source)

#else

typedef GDALDriver Driver;
typedef GDALDataset DataSource;

#define GET_DRIVER_BY_NAME(name) GetGDALDriverManager()->GetDriverByName(name)
#define CREATE_DATA_SOURCE(driver, filename, options) driver->Create(filename, 0, 0, 0, GDT_Unknown, options)
#define DESTROY_DATA_SOURCE(data_source) delete data_source

#endif

#endif // GDAL_SUPPORT_HPP
