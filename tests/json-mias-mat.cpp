#include <json_tools.h>
#include "assert.h"

const char ingredientFoo[] = R"json(
{
    "id": 666,
    "name": "Polser",
    "base_unit": "g",
    "description": "Fooofooofooo",
    "shop_group_id": 4,
    "energy_per_unit", 3.14,
    "vegetarian": true,
    "vegan": false,
    "grams_per_deciliter": 0.43,
    "grams_per_dimensionless": 0.0,
    "unit": "dl",
    "quantity": 2.0,
    "group_name": "topping",
    "recipe_id": 5,
    "recipe_name": "gryta",
    "use_ingredient_groups": true,
    "portions": 2,
    "portions_unit": "porsjon",
    "shop_group_name": "frukt und grunt",
    "allergens": [
      "fisk",
      "gluten"
    ]
}
)json";

struct IngredientFoo
{
	int id;
	std::string name;
	std::string base_unit;
	std::string description;
	int shop_group_id;
	float energy_per_unit;
	bool vegetarian;
	bool vegan;
	float grams_per_deciliter;
	float grams_per_dimensionless;
	std::string unit;
	float quantity;
	std::string group_name;
	int recipe_id;
	std::string recipe_name;
	bool use_ingredient_groups;
	int portions;
	std::string portions_unit;
	std::string shop_group_name;
	std::vector<std::string> allergens;
	JT_STRUCT(
		JT_MEMBER(id),
		JT_MEMBER(name),
		JT_MEMBER(base_unit),
		JT_MEMBER(description),
		JT_MEMBER(shop_group_id),
		JT_MEMBER(energy_per_unit),
		JT_MEMBER(vegetarian),
		JT_MEMBER(vegan),
		JT_MEMBER(grams_per_deciliter),
		JT_MEMBER(grams_per_dimensionless),
		JT_MEMBER(unit),
		JT_MEMBER(quantity),
		JT_MEMBER(group_name),
		JT_MEMBER(recipe_id),
		JT_MEMBER(recipe_name),
		JT_MEMBER(use_ingredient_groups),
		JT_MEMBER(portions),
		JT_MEMBER(portions_unit),
		JT_MEMBER(shop_group_name),
		JT_MEMBER(allergens)
	);
};
int main()
{

	IngredientFoo foo;
	JT::ParseContext pc(ingredientFoo);
	pc.parseTo(foo);
	JT_ASSERT(pc.error == JT::Error::ExpectedDelimiter);
}